// =========================== gameboard.cc ===========================
#include "gameboard.h"

#include <algorithm>
#include <random>
#include <sstream>

#include "player.h"
#include "tile.h"
#include "vertex.h"
#include "edge.h"
#include "boardview.h"

// ---------------- Construction ----------------

GameBoard::GameBoard(bool enhance,
                     const std::vector<int> &values,
                     const std::vector<Resources> &resources)
    : geeseTile{-1},
      boardValues(values),
      boardResources(resources) {

    if (values.size() != 19 || resources.size() != 19) {
        throw std::invalid_argument("GameBoard: values/resources must be size 19.");
    }

    initializePlayers();
    initializeBoardObjects(enhance);
    wireTopology();
}

GameBoard GameBoard::createRandom(bool enhance, unsigned seed) {
    // 18 real values; Netflix stored as 7 internally
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

    std::vector<Resources> resTypes = {
        Resources::Tutorial, Resources::Tutorial, Resources::Tutorial,
        Resources::Study, Resources::Study, Resources::Study,
        Resources::Caffeine, Resources::Caffeine, Resources::Caffeine, Resources::Caffeine,
        Resources::Lab, Resources::Lab, Resources::Lab, Resources::Lab,
        Resources::Lecture, Resources::Lecture, Resources::Lecture, Resources::Lecture,
        Resources::Netflix
    };

    std::mt19937 rng(seed);
    std::shuffle(values.begin(), values.end(), rng);
    std::shuffle(resTypes.begin(), resTypes.end(), rng);

    // Align values to resources; Netflix gets 7 placeholder
    std::vector<int> finalValues;
    finalValues.reserve(19);
    int k = 0;
    for (auto r : resTypes) {
        if (r == Resources::Netflix) finalValues.push_back(7);
        else finalValues.push_back(values[k++]);
    }

    return GameBoard{enhance, finalValues, resTypes};
}

void GameBoard::initializePlayers() {
    players.clear();
    players.reserve(4);
    players.emplace_back(std::make_unique<Player>(Colour::Blue));
    players.emplace_back(std::make_unique<Player>(Colour::Red));
    players.emplace_back(std::make_unique<Player>(Colour::Orange));
    players.emplace_back(std::make_unique<Player>(Colour::Yellow));
}

void GameBoard::initializeBoardObjects(bool enhance) {
    // Tiles
    tiles.clear();
    tiles.reserve(19);
    for (int i = 0; i < 19; ++i) {
        tiles.emplace_back(std::make_unique<Tile>(i, boardValues[i], boardResources[i]));
    }

    // Vertices + edges
    vertices.clear();
    edges.clear();
    vertices.reserve(54);
    edges.reserve(72);

    for (int i = 0; i < 54; ++i) {
        vertices.emplace_back(std::make_unique<Vertex>(i));
    }
    for (int i = 0; i < 72; ++i) {
        edges.emplace_back(std::make_unique<Edge>(i));
    }

    // View observes vertices/edges/geese
    view = std::make_unique<BoardView>(enhance, &boardValues, &boardResources);
    for (auto &v : vertices) v->attach(view.get());
    for (auto &e : edges) e->attach(view.get());
}

/**
 * wireTopology()
 * --------------
 * Fixed graph setup for the official board layout.
 *
 * Paste TWO tables from your reference implementation here:
 *
 * 1) tileVerts[19][6]  -> which 6 vertices touch each tile
 * 2) edgeEnds[72][2]   -> which 2 vertices each edge connects
 *
 * Then the adjacency wiring below works automatically.
 */
void GameBoard::wireTopology() {
    // ---------- 1) Tile -> 6 vertices ----------
    // TODO: paste full tileVerts mapping here
    // static const int tileVerts[19][6] = {
    //   { ... }, // tile 0
    //   ...
    // };
    //
    // for (int t = 0; t < 19; ++t) {
    //     for (int j = 0; j < 6; ++j) {
    //         int vid = tileVerts[t][j];
    //         tiles[t]->addVertex(vertices[vid].get());
    //     }
    // }

    // ---------- 2) Edge endpoints ----------
    // TODO: paste full edgeEnds mapping here
    // static const int edgeEnds[72][2] = {
    //   {a,b}, // edge 0
    //   ...
    // };
    //
    // for (int e = 0; e < 72; ++e) {
    //     int a = edgeEnds[e][0], b = edgeEnds[e][1];
    //     edges[e]->setEndpoints(vertices[a].get(), vertices[b].get());
    // }

    // ---------- 3) Build adjacency lists ----------
    // Once edgeEnds is pasted and setEndpoints is done:
    //
    // for (int e = 0; e < 72; ++e) {
    //     int a = edgeEnds[e][0], b = edgeEnds[e][1];
    //     Vertex *va = vertices[a].get();
    //     Vertex *vb = vertices[b].get();
    //
    //     va->addAdjacentEdge(edges[e].get());
    //     vb->addAdjacentEdge(edges[e].get());
    //
    //     va->addAdjacentVertex(vb);
    //     vb->addAdjacentVertex(va);
    // }
}

// ---------------- Core actions ----------------

void GameBoard::rollDice(Colour activePlayer) {
    Player *p = findPlayer(activePlayer);
    if (!p) throw std::runtime_error("rollDice: unknown player.");

    lastOutcome = RollOutcome{};
    lastOutcome.roll = p->rollDice();

    if (lastOutcome.roll == 7) {
        lastOutcome.isGeese = true;

        bool anyLost = false;
        for (auto &pl : players) {
            if (pl->numResources() >= 10) {
                anyLost |= pl->loseResourcesToGeese();
            }
        }
        lastOutcome.anyoneLostToGeese = anyLost;
        return;
    }

    // Normal distribution
    for (auto &t : tiles) {
        if (t->hasGeese()) continue;
        if (t->getValue() != lastOutcome.roll) continue;

        // Tile returns detailed gains
        auto gains = t->sendResources();
        lastOutcome.gains.insert(lastOutcome.gains.end(), gains.begin(), gains.end());
    }
}

void GameBoard::completeVertex(Colour playerColour, int vertexId) {
    Player *p = findPlayer(playerColour);
    Vertex *v = getVertex(vertexId);
    if (!p || !v) throw std::runtime_error("completeVertex: invalid args.");

    if (!v->canBeCompletedBy(playerColour)) {
        throw std::runtime_error("You cannot build here.");
    }
    if (!p->resourcesCheck(Assessment::Assignment)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    v->complete(playerColour);
    p->resourcesSpent(Assessment::Assignment);
    p->addVertex(vertexId, Assessment::Assignment);
}

void GameBoard::improveVertex(Colour playerColour, int vertexId) {
    Player *p = findPlayer(playerColour);
    Vertex *v = getVertex(vertexId);
    if (!p || !v) throw std::runtime_error("improveVertex: invalid args.");

    if (!v->canBeImprovedBy(playerColour)) {
        throw std::runtime_error("You cannot build here.");
    }

    Assessment cur = v->currentAssessment();
    Assessment nxt;

    if (cur == Assessment::Assignment) nxt = Assessment::Midterm;
    else if (cur == Assessment::Midterm) nxt = Assessment::Exam;
    else throw std::runtime_error("You cannot build here.");

    if (!p->resourcesCheck(nxt)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    v->improve();
    p->resourcesSpent(nxt);
    p->upgradeVertex(vertexId, nxt);
}

void GameBoard::achieveEdge(Colour playerColour, int edgeId) {
    Player *p = findPlayer(playerColour);
    Edge *e = getEdge(edgeId);
    if (!p || !e) throw std::runtime_error("achieveEdge: invalid args.");

    if (!e->canBeAchievedBy(playerColour)) {
        throw std::runtime_error("You cannot build here.");
    }
    if (!p->resourcesCheck(Assessment::Achievement)) {
        throw std::runtime_error("You do not have enough resources.");
    }

    e->achieve(playerColour);
    p->resourcesSpent(Assessment::Achievement);
    p->addEdge(edgeId);
}

void GameBoard::moveGeese(Colour /*activePlayer*/, int tileId) {
    Tile *newTile = getTile(tileId);
    if (!newTile || tileId == geeseTile) {
        throw std::invalid_argument("Invalid geese tile.");
    }

    if (geeseTile != -1) {
        tiles[geeseTile]->setGeese(false);
    }

    geeseTile = tileId;
    tiles[geeseTile]->setGeese(true);

    if (view) view->notifyGeese(geeseTile);
}

// ---------------- Printing / queries ----------------

void GameBoard::printBoard(std::ostream &out) const {
    if (view) view->render(out);
}

void GameBoard::printStatus(std::ostream &out) const {
    static const Colour order[4] = {
        Colour::Blue, Colour::Red, Colour::Orange, Colour::Yellow
    };
    for (Colour c : order) {
        const Player *p = findPlayer(c);
        if (p) p->printStatus(out);
    }
}

void GameBoard::printCriteria(Colour playerColour, std::ostream &out) const {
    const Player *p = findPlayer(playerColour);
    if (!p) throw std::runtime_error("printCriteria: unknown player.");
    p->printCriteria(out);
}

bool GameBoard::hasWinner() const {
    for (auto &p : players) {
        if (p->getPoints() >= 10) return true;
    }
    return false;
}

Colour GameBoard::getWinner() const {
    for (auto &p : players) {
        if (p->getPoints() >= 10) return p->getColour();
    }
    throw std::logic_error("No winner yet.");
}

std::vector<Colour> GameBoard::getStealableColoursOnTile(int tileId, Colour active) const {
    const Tile *t = getTile(tileId);
    if (!t) throw std::invalid_argument("Invalid tile.");

    return t->playersToStealFrom(active);
}

// ---------------- Private helpers ----------------

Player *GameBoard::findPlayer(Colour colour) {
    for (auto &p : players)
        if (p->getColour() == colour) return p.get();
    return nullptr;
}

const Player *GameBoard::findPlayer(Colour colour) const {
    for (auto &p : players)
        if (p->getColour() == colour) return p.get();
    return nullptr;
}

Vertex *GameBoard::getVertex(int id) {
    if (id < 0 || id >= (int)vertices.size()) return nullptr;
    return vertices[id].get();
}

const Vertex *GameBoard::getVertex(int id) const {
    if (id < 0 || id >= (int)vertices.size()) return nullptr;
    return vertices[id].get();
}

Edge *GameBoard::getEdge(int id) {
    if (id < 0 || id >= (int)edges.size()) return nullptr;
    return edges[id].get();
}

const Edge *GameBoard::getEdge(int id) const {
    if (id < 0 || id >= (int)edges.size()) return nullptr;
    return edges[id].get();
}

Tile *GameBoard::getTile(int id) {
    if (id < 0 || id >= (int)tiles.size()) return nullptr;
    return tiles[id].get();
}

const Tile *GameBoard::getTile(int id) const {
    if (id < 0 || id >= (int)tiles.size()) return nullptr;
    return tiles[id].get();
}
