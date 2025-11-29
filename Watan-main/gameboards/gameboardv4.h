// =========================== gameboard.h ===========================
#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <iosfwd>
#include <memory>
#include <vector>
#include <stdexcept>

// Enums per your UML/planning doc
#include "colour.h"
#include "resources.h"
#include "assessment.h"

class Player;
class Tile;
class Vertex;
class Edge;
class BoardView;

/**
 * GameBoard
 * ----------
 * Pure model / rules engine:
 *  - Owns Tiles, Vertices, Edges, Players, BoardView, geese location
 *  - Enforces gameplay rules (roll/build/improve/achieve/move geese)
 *  - NO direct I/O (no cin/cout)  [GameController handles printing/prompting]
 *
 * SaveManager handles serialization.
 */
class GameBoard {
public:
    // --- Controller-friendly structs (no I/O in GameBoard) ---
    struct ResourceGain {
        Colour who;
        Resources what;
        int amount;
    };

    struct RollOutcome {
        int roll = 0;
        bool isGeese = false;
        bool anyoneLostToGeese = false;
        std::vector<ResourceGain> gains;
    };

private:
    // --- Owned game state ---
    std::vector<std::unique_ptr<Tile>>   tiles;     // 19
    std::vector<std::unique_ptr<Vertex>> vertices;  // 54
    std::vector<std::unique_ptr<Edge>>   edges;     // 72
    std::vector<std::unique_ptr<Player>> players;   // 4
    std::unique_ptr<BoardView>           view;      // ASCII view (Observer)
    int geeseTile = -1;                             // -1 means off-board

    // Copies kept alive for BoardView
    std::vector<int> boardValues;
    std::vector<Resources> boardResources;

    RollOutcome lastOutcome;

public:
    // ---- Construction ----
    GameBoard(bool enhance,
              const std::vector<int> &values,
              const std::vector<Resources> &resources);

    // Random board per spec distribution
    static GameBoard createRandom(bool enhance, unsigned seed);

    // ---- Core actions ----
    void rollDice(Colour activePlayer);

    void completeVertex(Colour player, int vertexId);
    void improveVertex(Colour player, int vertexId);
    void achieveEdge(Colour player, int edgeId);

    void moveGeese(Colour activePlayer, int tileId);

    // ---- Queries / printing helpers ----
    void printBoard(std::ostream &out) const;
    void printStatus(std::ostream &out) const;
    void printCriteria(Colour player, std::ostream &out) const;

    bool hasWinner() const;
    Colour getWinner() const;

    int getGeeseTile() const { return geeseTile; }
    const RollOutcome &getLastRollOutcome() const { return lastOutcome; }

    std::vector<Colour> getStealableColoursOnTile(int tileId, Colour active) const;

    // Access for SaveManager / tests
    const std::vector<std::unique_ptr<Tile>>   &getTiles() const { return tiles; }
    const std::vector<std::unique_ptr<Vertex>> &getVertices() const { return vertices; }
    const std::vector<std::unique_ptr<Edge>>   &getEdges() const { return edges; }
    const std::vector<std::unique_ptr<Player>> &getPlayers() const { return players; }

private:
    // ---- Helpers ----
    void initializePlayers();
    void initializeBoardObjects(bool enhance);
    void wireTopology(); // fixed layout wiring (paste mapping here)

    Player *findPlayer(Colour c);
    const Player *findPlayer(Colour c) const;

    Vertex *getVertex(int id);
    const Vertex *getVertex(int id) const;

    Edge *getEdge(int id);
    const Edge *getEdge(int id) const;

    Tile *getTile(int id);
    const Tile *getTile(int id) const;

    void setupRows(int n,
        std::vector<std::vector<Vertex *>> &vertexRows,
        std::vector<std::vector<Edge   *>> &edgeRows);

    void wireRows(int n,
       const std::vector<std::vector<Vertex *>> &vertexRows,
       const std::vector<std::vector<Edge   *>> &edgeRows);
};

#endif
