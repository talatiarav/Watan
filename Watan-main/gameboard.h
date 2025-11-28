#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <iosfwd>
#include <memory>
#include <vector>

// Forward declarations of enums
enum class Colour;
enum class Resources;
enum class Assessment;

// Forward declarations of core classes
#include "player.h"
#include "tile.h"
#include "vertex.h"
#include "edge.h"
#include "boardview.h"

class GameBoard {
public:
    // Construct from explicit tile values/resources (size 19 each).
    GameBoard(bool enhance,
              const std::vector<int> &values,
              const std::vector<Resources> &resources);

    // Construct a random board with the standard Watan distribution.
    static GameBoard createRandom(bool enhance);

    void placeInitialAssignment(Colour playerColour, int vertexId);

    // Core game actions
    int rollDice(Colour activePlayer);
    void completeVertex(Colour playerColour, int vertexId);
    void improveVertex(Colour playerColour, int vertexId);
    void achieveEdge(Colour playerColour, int edgeId);
    void moveGeese(Colour activePlayer, int tileId);

    // Printing / queries
    void printBoard(std::ostream &out) const;
    void printStatus(std::ostream &out) const;
    void printCriteria(Colour playerColour, std::ostream &out) const;

    void printStatusFor(Colour playerColour, std::ostream &out) const;

    bool hasWinner() const;
    Colour getWinner() const;

    // Geese-steal helper
    std::vector<Colour> getStealableColoursOnTile(int tileId,
                                                  Colour active) const;

    // ----- Save/load helpers -----

    // Where the geese currently are (-1 if nowhere).
    int getGeeseTile() const { return geeseTile; }

    // Board layout in watan-savefile format:
    //  "<res0> <val0> <res1> <val1> ... <res18> <val18>"
    // with res codes: 0=Caff,1=Lab,2=Lect,3=Study,4=Tut,5=Netflix
    std::string encodeBoardLayoutForSave() const;

    // Load-only helpers: set edges/vertices from save without resource checks.
    void setRoadForLoad(Colour owner, int edgeId);
    void setResidenceForLoad(Colour owner, int vertexId, Assessment level);
    
    // For SaveManager: read-only access to players (fixed order: Blue, Red, Orange, Yellow).
    const std::vector<std::unique_ptr<Player>> &getPlayers() const { return players; }

    Player *getPlayer(Colour colour);
    const Player *getPlayer(Colour colour) const;



private:
    // Players are always in fixed order: Blue, Red, Orange, Yellow.
    std::vector<std::unique_ptr<Player>> players;

    // Board topology
    std::vector<std::unique_ptr<Tile>>   tiles;
    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<Edge>>   edges;

    // View / UI
    std::unique_ptr<BoardView> view;

    // Index of tile with the geese, or -1 if not placed yet
    int geeseTile;

    // Construction helpers
    void initializePlayers();
    void initializeBoardGraph(const std::vector<int> &values,
                              const std::vector<Resources> &resources,
                              bool enhance);

    // Topology helpers (ported from old Board::rowSetup / update / updateCriterionsInTile)
    void setupRows(int n,
                   std::vector<std::vector<Vertex *>> &vertexRows,
                   std::vector<std::vector<Edge   *>> &edgeRows);

    void wireRows(int n,
                  const std::vector<std::vector<Vertex *>> &vertexRows,
                  const std::vector<std::vector<Edge   *>> &edgeRows);

    // New: wire tiles to vertices using the same indexing pattern as old Board
    void wireTiles(int n);

    // New: helper analogous to criterionAdderHelper
    void addVerticesForTile(int &vertexIndex, int tileId);

    // Lookups
    Player *findPlayer(Colour colour);
    const Player *findPlayer(Colour colour) const;

    Vertex *getVertex(int vertexId);
    const Vertex *getVertex(int vertexId) const;

    Edge *getEdge(int edgeId);
    const Edge *getEdge(int edgeId) const;
};

#endif // GAMEBOARD_H
