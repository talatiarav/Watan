#include "gameboard.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <stdexcept>

// New enums
#include "colour.h"
#include "resources.h"
#include "assessment.h"

// New core classes (to be refactored from Student / Criterion / Goal / TextDisplay / Tile).
#include "player.h"
#include "tile.h"
#include "vertex.h"
#include "edge.h"
#include "boardview.h"

using std::cout;
using std::endl;

// ---------- Construction ----------

GameBoard::GameBoard(bool enhance,
                     const std::vector<int> &values,
                     const std::vector<Resources> &resources)
    : geeseTile{-1}
{
    if (values.size() != 19 || resources.size() != 19) {
        throw std::invalid_argument("GameBoard: values/resources vectors must be size 19.");
    }

    initializePlayers();
    initializeBoardGraph(values, resources, enhance);
}

GameBoard GameBoard::createRandom(bool enhance) {
    // Values: one 2, one 12, two each of 3–6 and 8–11
    std::vector<int> values = {
        2,
        3, 3,
        4, 4,
        5, 5,
        6, 6,
        8, 8,
        9, 9,
        10, 10,
        11, 11,
        12
    };

    // Resources: 3 TUTORIAL, 3 STUDY, 4 CAFFEINE, 4 LAB, 4 LECTURE, 1 NETFLIX
    std::vector<Resources> resTypes = {
        Resources::Tutorial, Resources::Tutorial, Resources::Tutorial,
        Resources::Study,    Resources::Study,    Resources::Study,
        Resources::Caffeine, Resources::Caffeine,
        Resources::Caffeine, Resources::Caffeine,
        Resources::Lab,      Resources::Lab,
        Resources::Lab,      Resources::Lab,
        Resources::Lecture,  Resources::Lecture,
        Resources::Lecture,  Resources::Lecture,
        Resources::Netflix
    };

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::random_shuffle(values.begin(), values.end());
    std::random_shuffle(resTypes.begin(), resTypes.end());

    return GameBoard{enhance, values, resTypes};
}

void GameBoard::initializePlayers() {
    players.clear();
    players.reserve(4);

    // Fixed order: Blue, Red, Orange, Yellow (spec and old Board).
    players.emplace_back(std::make_unique<Player>(Colour::Blue));
    players.emplace_back(std::make_unique<Player>(Colour::Red));
    players.emplace_back(std::make_unique<Player>(Colour::Orange));
    players.emplace_back(std::make_unique<Player>(Colour::Yellow));
}

void GameBoard::initializeBoardGraph(const std::vector<int> &values,
                                     const std::vector<Resources> &resources,
                                     bool enhance) {
    // 1) Build Tiles
    tiles.clear();
    tiles.reserve(19);
    for (int i = 0; i < 19; ++i) {
        tiles.emplace_back(std::make_unique<Tile>(values[i], resources[i]));
    }

    // 2) Build BoardView (ASCII display) from values/resources.
    // BoardView will internally copy these into its own string grids
    // (like TextDisplay did) and NOT keep references.
    view = std::make_unique<BoardView>(enhance, values, resources);

    // 3) Build Vertices & Edges and attach view as Observer.
    //
    // The canonical counts are:
    //  - 54 vertices (criteria)
    //  - 72 edges   (goals)
    vertices.clear();
    edges.clear();
    vertices.reserve(54);
    edges.reserve(72);

    for (int i = 0; i < 54; ++i) {
        auto v = std::make_unique<Vertex>(i);
        v->attach(view.get());
        vertices.emplace_back(std::move(v));
    }

    for (int i = 0; i < 72; ++i) {
        auto e = std::make_unique<Edge>(i);
        e->attach(view.get());
        edges.emplace_back(std::move(e));
    }

    // 4) Topology wiring (BIG TODO):
    //
    // Port the logic from the old Board helpers:
    //   - Board::rowSetup
    //   - Board::update
    //   - Board::updateCriterionsInTile
    //   - Board::updateCriterionsNeighbor
    //
    // to:
    //   - Connect the 54 Vertices and 72 Edges in the correct graph layout.
    //   - For each tile i:
    //       * call tiles[i]->addVertex(vPtr) for its 6 adjacent vertices
    //   - For each Vertex:
    //       * fill its adjacency lists of neighboring Vertices and Edges
    //   - For each Edge:
    //       * set its two endpoint Vertices
    //
    // Once that’s done, Vertex::canBeCompletedBy, canBeImprovedBy,
    // Edge::canBeAchievedBy, and Tile::sendResources will all have
    // enough context to enforce the rules.
}

// ---------- Core game actions ----------

void GameBoard::rollDice(Colour activePlayer) {
    Player *p = findPlayer(activePlayer);
    if (!p) {
        throw std::runtime_error("rollDice: unknown player colour.");
    }

    int roll = p->rollDice();

    if (roll == 7) {
        // Geese logic: any player with >= 10 resources loses half.
        bool anyLost = false;
        for (const auto &pl : players) {
            if (pl->numResources() >= 10) {
                anyLost = true;
                break;
            }
        }

        if (!anyLost) {
            cout << "No students lost resources to the GEESE." << endl;
        } else {
            for (auto &pl : players) {
                if (pl->numResources() >= 10) {
                    pl->loseResourcesToGeese();
                }
            }
        }

        // GameController must now ask the user where to move the geese and
        // then call moveGeese(...) and handle stealing.
        return;
    }

    // Non-7 roll: resource distribution.
    bool sent = false;
    for (auto &t : tiles) {
        if (t->getValue() == roll) {
            if (t->sendResources()) {
                sent = true;
            }
        }
    }

    if (!sent) {
        cout << "No students gained resources." << endl;
    }
}

void GameBoard::completeVertex(Colour playerColour, int vertexId) {
    Player *p = findPlayer(playerColour);
    if (!p) {
        throw std::runtime_error("completeVertex: unknown player colour.");
    }

    Vertex *v = getVertex(vertexId);
    if (!v) {
        throw std::runtime_error("completeVertex: invalid vertex id.");
    }

    if (!v->canBeCompletedBy(playerColour)) {
        throw std::runtime_error("You cannot build here.");
    }

    if (!p->resourcesCheck(Assessment::Assignment)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    v->complete(p);
    p->resourcesSpent(Assessment::Assignment);
    p->addVertex(v);
}

void GameBoard::improveVertex(Colour playerColour, int vertexId) {
    Player *p = findPlayer(playerColour);
    if (!p) {
        throw std::runtime_error("improveVertex: unknown player colour.");
    }

    Vertex *v = getVertex(vertexId);
    if (!v) {
        throw std::runtime_error("improveVertex: invalid vertex id.");
    }

    if (!v->canBeImprovedBy(playerColour)) {
        throw std::runtime_error("You cannot build here.");
    }

    Assessment current = v->currentAssessment();
    Assessment next;

    switch (current) {
        case Assessment::Assignment:
            next = Assessment::Midterm;
            break;
        case Assessment::Midterm:
            next = Assessment::Exam;
            break;
        default:
            // None or Exam can't be improved
            throw std::runtime_error("You cannot build here.");
    }

    if (!p->resourcesCheck(next)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    v->improve();
    p->resourcesSpent(next);
}

void GameBoard::achieveEdge(Colour playerColour, int edgeId) {
    Player *p = findPlayer(playerColour);
    if (!p) {
        throw std::runtime_error("achieveEdge: unknown player colour.");
    }

    Edge *e = getEdge(edgeId);
    if (!e) {
        throw std::runtime_error("achieveEdge: invalid edge id.");
    }

    if (!e->canBeAchievedBy(playerColour)) {
        throw std::runtime_error("You cannot build here.");
    }

    if (!p->resourcesCheck(Assessment::Achievement)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    e->achieve(p);
    p->resourcesSpent(Assessment::Achievement);
    p->addEdge(e);
}

void GameBoard::moveGeese(Colour /*activePlayer*/, int tileId) {
    if (tileId < 0 || tileId >= static_cast<int>(tiles.size()) ||
        tileId == geeseTile) {
        throw std::invalid_argument("Invalid geese tile.");
    }

    if (geeseTile != -1) {
        tiles[geeseTile]->toggleGeese();
    }

    geeseTile = tileId;
    tiles[geeseTile]->toggleGeese();

    if (view) {
        view->notifyGeese(geeseTile);
    }
}

// ---------- Printing / queries ----------

void GameBoard::printBoard(std::ostream &out) const {
    if (view) {
        view->render(out);
    }
}

void GameBoard::printStatus(std::ostream &out) const {
    const Colour order[4] = {
        Colour::Blue, Colour::Red, Colour::Orange, Colour::Yellow
    };

    for (Colour c : order) {
        const Player *p = findPlayer(c);
        if (p) {
            p->printStatus(out);
        }
    }
}

void GameBoard::printCriteria(Colour playerColour, std::ostream &out) const {
    const Player *p = findPlayer(playerColour);
    if (!p) {
        throw std::runtime_error("printCriteria: unknown player colour.");
    }
    p->printCriteria(out);
}

bool GameBoard::hasWinner() const {
    for (const auto &p : players) {
        if (p->getPoints() >= 10) {
            return true;
        }
    }
    return false;
}

Colour GameBoard::getWinner() const {
    for (const auto &p : players) {
        if (p->getPoints() >= 10) {
            return p->getColour();
        }
    }
    throw std::logic_error("getWinner called but no player has 10 points.");
}

std::vector<Colour> GameBoard::getStealableColoursOnTile(int tileId,
                                                         Colour active) const {
    if (tileId < 0 || tileId >= static_cast<int>(tiles.size())) {
        throw std::invalid_argument("Invalid tileId.");
    }

    std::string names = tiles[tileId]->playersToStealFrom(active);
    std::vector<Colour> result;

    std::istringstream iss(names);
    std::string token;
    while (std::getline(iss, token, ',')) {
        if (token == "Blue") {
            result.push_back(Colour::Blue);
        } else if (token == "Red") {
            result.push_back(Colour::Red);
        } else if (token == "Orange") {
            result.push_back(Colour::Orange);
        } else if (token == "Yellow") {
            result.push_back(Colour::Yellow);
        }
    }

    return result;
}

// ---------- Private helpers ----------

Player *GameBoard::findPlayer(Colour colour) {
    for (auto &p : players) {
        if (p->getColour() == colour) {
            return p.get();
        }
    }
    return nullptr;
}

const Player *GameBoard::findPlayer(Colour colour) const {
    for (const auto &p : players) {
        if (p->getColour() == colour) {
            return p.get();
        }
    }
    return nullptr;
}

Vertex *GameBoard::getVertex(int vertexId) {
    if (vertexId < 0 || vertexId >= static_cast<int>(vertices.size())) {
        return nullptr;
    }
    return vertices[vertexId].get();
}

const Vertex *GameBoard::getVertex(int vertexId) const {
    if (vertexId < 0 || vertexId >= static_cast<int>(vertices.size())) {
        return nullptr;
    }
    return vertices[vertexId].get();
}

Edge *GameBoard::getEdge(int edgeId) {
    if (edgeId < 0 || edgeId >= static_cast<int>(edges.size())) {
        return nullptr;
    }
    return edges[edgeId].get();
}

const Edge *GameBoard::getEdge(int edgeId) const {
    if (edgeId < 0 || edgeId >= static_cast<int>(edges.size())) {
        return nullptr;
    }
    return edges[edgeId].get();
}
