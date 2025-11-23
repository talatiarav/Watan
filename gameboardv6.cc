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

// New core classes
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
    tiles.reserve(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        // you’ll likely want to add a tile ID parameter later
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

    // 4) Wire tiles to vertices using the same indexing pattern as the old Board.
    //    This gives each Tile its 6 surrounding vertices and lets each Vertex
    //    know which Tiles it touches.
    wireTiles(n);

    geeseTile = -1;
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

// ---------- Topology helpers (rows / edges) ----------

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
            // even rows: k edges, 2k vertices
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
            // odd rows: 2k edges, no vertices
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
    int temp = n; // number of edges in patternedRow

    // Middle band
    for (int i = patternedRow; i < secondPatternedRow; ++i) {
        if (i % 2 == 0) {
            // even rows: temp edges, (2n+2) vertices
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
            // odd rows: (2n+2) edges, no vertices
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
            // odd rows: 2k edges
            for (int j = 0; j < (2 * k); ++j) {
                int edgeId = static_cast<int>(edges.size());
                auto e = std::make_unique<Edge>(edgeId);
                e->attach(view.get());
                edgeRows[i].push_back(e.get());
                edges.emplace_back(std::move(e));
            }
        } else {
            // even rows: k edges, 2k vertices
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
            int cCounter = (size == n ? 1 : 0);
            for (int g = 0; g < size; ++g) {
                Edge   *e  = edgeRows[i][g];
                Vertex *v1 = vertexRows[i][cCounter++];
                Vertex *v2 = vertexRows[i][cCounter++];
                connectEdge(e, v1, v2);
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

// ---------- Tile–vertex wiring (ported from updateCriterionsInTile) ----------

void GameBoard::addVerticesForTile(int &vertexIndex, int tileId) {
    // helper analogous to Board::criterionAdderHelper
    for (int i = 0; i < 2; ++i) {
        if (tileId < 0 || tileId >= static_cast<int>(tiles.size())) {
            throw std::out_of_range("addVerticesForTile: tileId out of range");
        }
        if (vertexIndex < 0 || vertexIndex >= static_cast<int>(vertices.size())) {
            throw std::out_of_range("addVerticesForTile: vertexIndex out of range");
        }

        Tile   *t = tiles[tileId].get();
        Vertex *v = vertices[vertexIndex].get();

        t->addVertex(v);
        v->addAdjacentTile(t);

        ++vertexIndex;
    }
}

void GameBoard::wireTiles(int n) {
    // This function mirrors the old Board::updateCriterionsInTile(n)
    // but uses Vertex / Tile instead of Criterion / Tile.

    int patternStartsAt = (n * (n + 1)) / 2; // tile index
    double secondPattern = ((n * n) / 2.0) + ((3 * n) / 2.0) + 1;

    int two_n  = 2 * patternStartsAt;              // counter for first band
    int two_n2 = static_cast<int>(2 * secondPattern + 1); // counter for second band
    int start;                                     // remembers last "starting" in first band

    // Main pattern bands
    for (int k = 0; k < n + 1; ++k) {
        int starting = patternStartsAt;
        if (k != 0) {
            starting += (2 * n + 1) * k;
        }
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < n + 1; ++j) {
                addVerticesForTile(two_n, starting + j);
            }
        }
        two_n -= 2 * n + 2;
        start = starting;
    }

    for (int k = 0; k < n; ++k) {
        int starting = static_cast<int>(secondPattern);
        if (k != 0) {
            starting += (2 * n + 1) * k;
        }
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < n; ++j) {
                addVerticesForTile(two_n2, starting + j);
            }
            two_n2 += 2;
        }
        two_n2 -= 2 * n + 2;
    }

    // FIRST END CASE (top cap)

    std::vector<std::vector<int>> row; // helper for top rows of tiles
    row.resize(n + 1);
    int tileNo = 0;
    for (int k = 0; k < n + 1; ++k) {
        for (int j = 0; j < k + 1; ++j) {
            row[k].emplace_back(tileNo);
            ++tileNo;
        }
    }

    int vert = 0;
    for (int k = 0; k < n + 1; ++k) {
        int vert2 = vert + 1;
        if (k == n) {
            int vert3 = vert2 + 1;
            for (int t : row[k - 1]) { // row where pattern starts
                addVerticesForTile(vert2, t);
            }
            if (k > 1) {
                for (int t : row[k - 2]) {
                    addVerticesForTile(vert3, t);
                }
                vert3 += 3;
                for (int t : row[k - 1]) {
                    addVerticesForTile(vert3, t);
                }
                break;
            }
            vert2 += 2;
            for (int t : row[k - 1]) {
                addVerticesForTile(vert2, t);
            }
            break;
        }

        for (int t : row[k]) {
            addVerticesForTile(vert, t);
        }

        if (k >= 2) {
            int vert3 = vert2 + 1;
            for (int t : row[k - 1]) {
                addVerticesForTile(vert2, t);
            }
            for (int t : row[k - 2]) {
                addVerticesForTile(vert3, t);
            }
        } else if (k == 1) {
            for (int t : row[k - 1]) {
                addVerticesForTile(vert2, t);
            }
        }
    }

    // SECOND END CASE (bottom cap)

    std::vector<std::vector<int>> bottomRow;
    bottomRow.resize(n);

    int secondEndCase = start + n + 1;
    int counter = n;
    int start2 = two_n + 1;
    int start3 = start2 + 1;

    for (int k = n - 1; k >= 0; --k) {
        for (int j = 0; j < counter; ++j) {
            bottomRow[k].emplace_back(secondEndCase);
            ++secondEndCase;
        }
        --counter;
    }

    int temp = two_n - (2 * n) - 1;
    // special bottom row case
    for (int t : bottomRow[n - 1]) {
        addVerticesForTile(temp,   t);
        addVerticesForTile(start2, t);
    }

    if (n - 2 >= 0) {
        for (int t : bottomRow[n - 2]) {
            addVerticesForTile(start3, t);
        }
    }
    ++start2;

    int temp2 = start2 + 1; // for k - 1
    for (int k = n - 1; k >= 0; --k) {
        int temp3 = temp2 + 1; // for k - 2
        for (int t : bottomRow[k]) {
            addVerticesForTile(start2, t);
        }
        if (k - 1 >= 0) {
            for (int t : bottomRow[k - 1]) {
                addVerticesForTile(temp2, t);
            }
            if (k - 2 >= 0) {
                for (int t : bottomRow[k - 2]) {
                    addVerticesForTile(temp3, t);
                }
            }
        }
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
