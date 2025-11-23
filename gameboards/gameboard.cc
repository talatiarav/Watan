#include "gameboard.h"

#include <algorithm>    // std::random_shuffle
#include <cstdlib>      // std::rand, std::srand
#include <ctime>
#include <iostream>
#include <sstream>

// These headers are for the new classes you will create
// (ported from existing student/criterion/goal/textdisplay/tile code).
#include "player.h"
#include "tile.h"
#include "vertex.h"
#include "edge.h"
#include "boardview.h"

// And enums (you can put these in separate headers):
#include "colour.h"
#include "resources.h"
#include "assessment.h"

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
    // Values: one 2, one 12, two 3–6, two 8–11 
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
        Resources::Caffeine, Resources::Caffeine, Resources::Caffeine, Resources::Caffeine,
        Resources::Lab,      Resources::Lab,      Resources::Lab,      Resources::Lab,
        Resources::Lecture,  Resources::Lecture,  Resources::Lecture,  Resources::Lecture,
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

    // Fixed player order: Blue, Red, Orange, Yellow (same as spec) :contentReference[oaicite:19]{index=19}
    players.emplace_back(std::make_unique<Player>(Colour::Blue));
    players.emplace_back(std::make_unique<Player>(Colour::Red));
    players.emplace_back(std::make_unique<Player>(Colour::Orange));
    players.emplace_back(std::make_unique<Player>(Colour::Yellow));
}

void GameBoard::initializeBoardGraph(const std::vector<int> &values,
                                     const std::vector<Resources> &resources,
                                     bool enhance) {
    // Build Tiles
    tiles.clear();
    tiles.reserve(19);
    for (int i = 0; i < 19; ++i) {
        tiles.emplace_back(std::make_unique<Tile>(values[i], resources[i]));
    }

    // Build BoardView (ASCII text display), as a port of TextDisplay.
    // TextDisplay::TextDisplay(bool, vector<int>*, vector<Resource>*) 
    // We keep local copies of values/resources inside GameBoard to pass pointers.

  // Need to fix this --> will crash once the vectors are freed 
    auto valuesCopy    = std::make_shared<std::vector<int>>(values);
    auto resourcesCopy = std::make_shared<std::vector<Resources>>(resources);

    view = std::make_unique<BoardView>(enhance, valuesCopy.get(), resourcesCopy.get());

    // Build vertices/edges topology and hook them up to tiles & view.
    //
    // This is where you port the topology-building logic from:
    //  - Board::rowSetup
    //  - Board::update
    //  - Board::updateCriterionsInTile
    //  - Board::updateCriterionsNeighbor :contentReference[oaicite:21]{index=21}
    //
    // to create 54 Vertex objects and 72 Edge objects, and:
    //  - attach BoardView as an Observer to every Vertex and Edge
    //  - call Tile::addVertex for each vertex touching that tile
    //
    // For now, we just reserve space and leave detailed wiring to your next step.
    vertices.clear();
    edges.clear();
    vertices.reserve(54);
    edges.reserve(72);

    for (int i = 0; i < 54; ++i) {
        vertices.emplace_back(std::make_unique<Vertex>(i));
        // Vertex should inherit from Subject and support attach(view.get()).
        vertices.back()->attach(view.get());
    }

    for (int i = 0; i < 72; ++i) {
        edges.emplace_back(std::make_unique<Edge>(i));
        edges.back()->attach(view.get());
    }

    // TODO: Port the full adjacency wiring from the old Board helpers:
    //  - assign which vertices belong to which tiles
    //  - build adjacency lists between vertices and edges
    //  - ensure Vertex knows its adjacentVertices and adjacentEdges
    //  - ensure Edge knows its two endpoint vertices
    //
    // This keeps GameBoard as the owner/orchestrator, but moves the heavy
    // topology logic out of main().
}

// ---------- Core game actions ----------

void GameBoard::rollDice(Colour activePlayer) {
    Player *p = findPlayer(activePlayer);
    if (!p) {
        throw std::runtime_error("rollDice: unknown player colour.");
    }

    int roll = p->rollDice();

    if (roll == 7) {
        // Geese: any student with 10 or more resources loses half (rounded down).
        bool anyoneLost = false;
        for (const auto &ptr : players) {
            if (ptr->numResources() >= 10) {
                anyoneLost = true;
                break;
            }
        }

        if (!anyoneLost) {
            cout << "No students lost resources to the GEESE." << endl;
        } else {
            for (auto &ptr : players) {
                ptr->loseResourcesToGeese();
            }
        }

        // GameController is responsible for:
        //  - calling moveGeese(activePlayer, tileId)
        //  - then deciding whom to steal from and calling Player::stealFrom.
        return;
    }

    // Normal resource distribution case.
    bool anyGained = false;
    for (auto &t : tiles) {
        if (t->getValue() == roll && !t->hasGeese()) {
            if (t->sendResources()) {
                anyGained = true;
            }
        }
    }

    if (!anyGained) {
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

    // Assignment cost: one CAFFEINE, one LAB, one LECTURE, one TUTORIAL 
    if (!p->resourcesCheck(Assessment::Assignment)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    v->complete(playerColour);
    p->resourcesSpent(Assessment::Assignment);
    // Player should internally track owned vertices/points; GameBoard does not.
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
            // None or Exam can't be improved.
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

    // Achievement cost: one STUDY and one TUTORIAL (Assessment::Achievement) 
    if (!p->resourcesCheck(Assessment::Achievement)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    e->achieve(playerColour);
    p->resourcesSpent(Assessment::Achievement);
}

void GameBoard::moveGeese(Colour /*activePlayer*/, int tileId) {
    if (tileId < 0 || tileId >= static_cast<int>(tiles.size()) || tileId == geeseTile) {
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
    // Always print in Blue, Red, Orange, Yellow order, per spec :contentReference[oaicite:24]{index=24}.
    const Colour order[4] = {Colour::Blue, Colour::Red, Colour::Orange, Colour::Yellow};

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

std::vector<Colour> GameBoard::getStealableColoursOnTile(int tileId, Colour active) const {
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
