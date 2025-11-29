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

void GameBoard::initializeBoardGraph(const std::vector<int> &values,
    const std::vector<Resources> &resources,
    bool enhance) {
    // 1) Build Tiles
    tiles.clear();
    tiles.reserve(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        tiles.emplace_back(std::make_unique<Tile>(values[i], resources[i]));
    }

    // 2) Build BoardView
    view = std::make_unique<BoardView>(enhance, values, resources);

    // 3) Use rowSetup/update-style logic for vertices & edges.
    //    For the standard Watan board, layers n = 2
    //    since 3n^2 + 3n + 1 = 19 tiles ⇒ n = 2.
    const int n = 2;

    // These mirror the old vector<vector<Criterion>> / vector<vector<Goal>>,
    // but now hold Vertex* and Edge* instead of unique_ptrs.
    std::vector<std::vector<Vertex *>> vertexRows;
    std::vector<std::vector<Edge   *>> edgeRows;

    // These will fill GameBoard::vertices and GameBoard::edges.
    vertices.clear();
    edges.clear();

    setupRows(n, vertexRows, edgeRows);
    wireRows(n, vertexRows, edgeRows);

    // 4) TODO: Port updateCriterionsInTile logic:
    //    - for each tile, determine its 6 vertices (using the same pattern as
    //      the old code) and then:
    //          tile->addVertex(v);
    //          v->addAdjacentTile(tile);
    //
    //    Once you do that, resource distribution and adjacency-based rules
    //    will be fully wired.

    geeseTile = -1;
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

void GameBoard::setupRows(int n,
    std::vector<std::vector<Vertex *>> &vertexRows,
    std::vector<std::vector<Edge   *>> &edgeRows) {
    const int rows = 8 * n + 5;

    vertexRows.clear();
    edgeRows.clear();
    vertexRows.resize(rows);
    edgeRows.resize(rows);

    int patternedRow = (2 * n) + 2;

    int k = 1; // horizontal goal counter

    // First band of rows (top)
    for (int i = 0; i < patternedRow; ++i) {
        if (i % 2 == 0) {
        // even rows: k goals, 2k vertices
            for (int j = 0; j < k; ++j) {
                int edgeId = static_cast<int>(edges.size());
                auto e = std::make_unique<Edge>(edgeId);
                e->attach(view.get());
                edgeRows[i].push_back(e.get());
                edges.emplace_back(std::move(e));
            }

            for (int j = 0; j < (2 * k); ++j) {
                int vertexId = static_cast<int>(vertices.size());
                auto v = std::make_unique<Vertex>(vertexId);
                v->attach(view.get());
                vertexRows[i].push_back(v.get());
                vertices.emplace_back(std::move(v));
            }
        } else {
            // odd rows: 2k goals, no vertices
            for (int j = 0; j < (2 * k); ++j) {
            int edgeId = static_cast<int>(edges.size());
            auto e = std::make_unique<Edge>(edgeId);
            e->attach(view.get());
            edgeRows[i].push_back(e.get());
            edges.emplace_back(std::move(e));
            }
            ++k;
        }
    }

    int secondPatternedRow = (6 * n) + 3;
    int temp = n; // number of goals in patternedRow

    // Middle band
    for (int i = patternedRow; i < secondPatternedRow; ++i) {
        if (i % 2 == 0) {
            // even rows: temp goals, (2n+2) vertices
            for (int j = 0; j < temp; ++j) {
                int edgeId = static_cast<int>(edges.size());
                auto e = std::make_unique<Edge>(edgeId);
                e->attach(view.get());
                edgeRows[i].push_back(e.get());
                edges.emplace_back(std::move(e));
            }

            if (temp == n) { ++temp; }
            else           { --temp; }

            for (int j = 0; j < ((2 * n) + 2); ++j) {
                int vertexId = static_cast<int>(vertices.size());
                auto v = std::make_unique<Vertex>(vertexId);
                v->attach(view.get());
                vertexRows[i].push_back(v.get());
                vertices.emplace_back(std::move(v));
            }
        } else {
        // odd rows: (2n+2) goals, no vertices
            for (int j = 0; j < ((2 * n) + 2); ++j) {
                int edgeId = static_cast<int>(edges.size());
                auto e = std::make_unique<Edge>(edgeId);
                e->attach(view.get());
                edgeRows[i].push_back(e.get());
                edges.emplace_back(std::move(e));
            }
        }
    }

    // Last band (bottom)
    --k; // horizontal goal counter
    for (int i = secondPatternedRow; i < rows; ++i) {
        if (i % 2 != 0) {
        // odd rows: 2k goals
            for (int j = 0; j < (2 * k); ++j) {
                int edgeId = static_cast<int>(edges.size());
                auto e = std::make_unique<Edge>(edgeId);
                e->attach(view.get());
                edgeRows[i].push_back(e.get());
                edges.emplace_back(std::move(e));
            }
        } else {
            // even rows: k goals, 2k vertices
            for (int j = 0; j < k; ++j) {
                int edgeId = static_cast<int>(edges.size());
                auto e = std::make_unique<Edge>(edgeId);
                e->attach(view.get());
                edgeRows[i].push_back(e.get());
                edges.emplace_back(std::move(e));
            }

            for (int j = 0; j < (2 * k); ++j) {
                int vertexId = static_cast<int>(vertices.size());
                auto v = std::make_unique<Vertex>(vertexId);
                v->attach(view.get());
                vertexRows[i].push_back(v.get());
                vertices.emplace_back(std::move(v));
            }
            --k;
        }
    }
}

void GameBoard::wireRows(int n,
    const std::vector<std::vector<Vertex *>> &vertexRows,
    const std::vector<std::vector<Edge   *>> &edgeRows) {
    auto connectEdge = [](Edge *e, Vertex *v1, Vertex *v2) {
        if (!e || !v1 || !v2) return;

        e->setEndpoints(v1, v2);

        v1->addIncidentEdge(e);
        v2->addIncidentEdge(e);

        v1->addNeighbour(v2);
        v2->addNeighbour(v1);
    };

    const int rows = static_cast<int>(vertexRows.size());
    int patternedRow = (2 * n) + 2;

    // Top band
    int goalCounter = 1;
    for (int i = 0; i < patternedRow; ++i) {
        if (i % 2 == 0) {
            int c = 0;
            for (int g = 0; g < goalCounter; ++g) {
            Edge   *e  = edgeRows[i][g];
            Vertex *v1 = vertexRows[i][c++];
            Vertex *v2 = vertexRows[i][c++];
            connectEdge(e, v1, v2);
            }
            ++goalCounter;
        } else {
            int size = static_cast<int>(edgeRows[i].size());
            for (int g = 0; g < size; ++g) {
                Edge   *e  = edgeRows[i][g];
                Vertex *v1 = vertexRows[i - 1][g];
                Vertex *v2;

                if (i == patternedRow - 1) {
                    v2 = vertexRows[i + 1][g];
                } else {
                    v2 = vertexRows[i + 1][g + 1];
                }
                connectEdge(e, v1, v2);
            }
        }
    }

    // Middle band
    int secondPatternedRow = (6 * n) + 3;
    for (int i = patternedRow; i <= secondPatternedRow; ++i) {
        int size = static_cast<int>(edgeRows[i].size());
        if (i % 2 == 0) {
            if (size == n) {
                int cCounter = 1;
                for (int g = 0; g < size; ++g) {
                    Edge   *e  = edgeRows[i][g];
                    Vertex *v1 = vertexRows[i][cCounter++];
                    Vertex *v2 = vertexRows[i][cCounter++];
                    connectEdge(e, v1, v2);
                }
            } else {
                int cCounter = 0;
                for (int g = 0; g < size; ++g) {
                    Edge   *e  = edgeRows[i][g];
                    Vertex *v1 = vertexRows[i][cCounter++];
                    Vertex *v2 = vertexRows[i][cCounter++];
                    connectEdge(e, v1, v2);
                }
            }
        } else {
            for (int g = 0; g < size; ++g) {
                Edge   *e  = edgeRows[i][g];
                Vertex *v1 = vertexRows[i - 1][g];
                Vertex *v2 = vertexRows[i + 1][g];
                connectEdge(e, v1, v2);
            }
        }
    }

    // Bottom band
    int gCounter = n + 1;
    for (int i = secondPatternedRow + 1; i < rows; ++i) {
        int size = static_cast<int>(edgeRows[i].size());
        if (i % 2 == 0) {
            int c = 0;
            for (int g = 0; g < gCounter; ++g) {
                Edge   *e  = edgeRows[i][g];
                Vertex *v1 = vertexRows[i][c++];
                Vertex *v2 = vertexRows[i][c++];
                connectEdge(e, v1, v2);
            }
        --gCounter;
        } else {
            for (int g = 0; g < size; ++g) {
                Edge   *e  = edgeRows[i][g];
                Vertex *v1 = vertexRows[i - 1][g + 1];
                Vertex *v2 = vertexRows[i + 1][g];
                connectEdge(e, v1, v2);
            }
        }
    }
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
