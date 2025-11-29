export module GameBoard;

import <iosfwd>;
import <memory>;
import <vector>;
import <string>;

import Colour;
import Resources;
import Assessment;
import Player;
import Tile;
import Vertex;
import Edge;
import BoardView;


// Manages all core game logic and state for the Watan board.
// Tracks tiles, vertices, edges, resources, geese position, and player actions.
// Provides the rules for completing/improving criteria, achieving goals,
// distributing resources, moving geese, and updating the BoardView.


export class GameBoard {
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
    // with res codes: 0=Caff,1=Lab,2=Lect,3=Study,4=Tut,5=nflx
    std::string encodeBoardLayoutForSave() const;

    // Load-only helpers: set edges/vertices from save without resource checks.
    void setEdgeForLoad(Colour owner, int edgeId);
    void setVertexForLoad(Colour owner, int vertexId, Assessment level);
    
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

    void setupRows(int n,
                   std::vector<std::vector<Vertex *>> &vertexRows,
                   std::vector<std::vector<Edge   *>> &edgeRows);

    void wireRows(int n,
                  const std::vector<std::vector<Vertex *>> &vertexRows,
                  const std::vector<std::vector<Edge   *>> &edgeRows);

    void wireTiles(int n);

    void addVerticesForTile(int &vertexIndex, int tileId);

    // Lookups
    Player *findPlayer(Colour colour);
    const Player *findPlayer(Colour colour) const;

    Vertex *getVertex(int vertexId);
    const Vertex *getVertex(int vertexId) const;

    Edge *getEdge(int edgeId);
    const Edge *getEdge(int edgeId) const;
};
