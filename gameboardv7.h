#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "colour.h"
#include "resources.h"

class Player;
class Tile;
class Vertex;
class Edge;
class BoardView;

// GameBoard is the central model: it owns tiles, vertices, edges, and players.
// It also owns the view (BoardView) that observes changes on vertices/edges/geese.
class GameBoard {
public:
    // Construct from explicit tile values/resources (typically for -board or -load).
    GameBoard(bool enhance,
              const std::vector<int> &values,
              const std::vector<Resources> &resources);

    // Standard random board (values/resources shuffled as per spec).
    static GameBoard createRandom(bool enhance);

    // Rolls the dice for the given player, performs all *automatic* consequences
    // (7 -> resource loss; non-7 -> resource distribution), prints the standard
    // messages, and returns the rolled value.
    int rollDice(Colour activePlayer);

    // Building / improving on the graph
    void completeVertex(Colour playerColour, int vertexId);
    void improveVertex(Colour playerColour, int vertexId);
    void achieveEdge(Colour playerColour, int edgeId);

    // Move geese to tileId (GameController ensures tileId is chosen by the user
    // and that stealing, if any, is handled after this call).
    void moveGeese(Colour activePlayer, int tileId);

    // Printing / queries
    void printBoard(std::ostream &out) const;
    void printStatus(std::ostream &out) const;
    void printCriteria(Colour playerColour, std::ostream &out) const;

    bool hasWinner() const;
    Colour getWinner() const;

    // Used by GameController when prompting for stealing targets after a 7.
    std::vector<Colour> getStealableColoursOnTile(int tileId,
                                                  Colour active) const;

    // ---- Save/load helpers (for SaveManager) ----

    // Encodes the board layout in the spec format:
    //  "<res0> <val0> <res1> <val1> ... <res18> <val18>"
    // where res is:
    //   0 = CAFFEINE, 1 = LAB, 2 = LECTURE, 3 = STUDY, 4 = TUTORIAL, 5 = NETFLIX.
    std::string encodeBoardLayoutForSave() const;

    // Where the geese currently are (-1 if nowhere).
    int getGeeseTile() const { return geeseTile; }

    // Access to players for SaveManager (to call per-player encodeForSave).
    const std::vector<std::unique_ptr<Player>> &getPlayers() const {
        return players;
    }
    std::vector<std::unique_ptr<Player>> &getPlayers() {
        return players;
    }

private:
    int geeseTile; // -1 means not placed yet

    std::vector<std::unique_ptr<Player>> players;
    std::vector<std::unique_ptr<Tile>> tiles;
    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<Edge>> edges;

    std::unique_ptr<BoardView> view;

    // Construction helpers
    void initializePlayers();
    void initializeBoardGraph(const std::vector<int> &values,
                              const std::vector<Resources> &resources,
                              bool enhance);

    // Topology construction (ported from rowSetup/update logic).
    void setupRows(int n,
                   std::vector<std::vector<Vertex *>> &vertexRows,
                   std::vector<std::vector<Edge *>> &edgeRows);
    void wireRows(int n,
                  const std::vector<std::vector<Vertex *>> &vertexRows,
                  const std::vector<std::vector<Edge *>> &edgeRows);

    // Local helpers
    Player *findPlayer(Colour colour);
    const Player *findPlayer(Colour colour) const;

    Vertex *getVertex(int vertexId);
    const Vertex *getVertex(int vertexId) const;

    Edge *getEdge(int edgeId);
    const Edge *getEdge(int edgeId) const;
};

#endif // GAMEBOARD_H
