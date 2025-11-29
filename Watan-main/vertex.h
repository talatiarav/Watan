export module Vertex;

import <vector>;

import Assessment;
import Colour;

class Player;
class Edge;
class Tile;
class BoardView;

// Represents a course criterion location on the board.
// Tracks its owner, current assessment level, and neighbouring edges/vertices.
// Used to check whether a player can complete or upgrade a criterion,
// and notifies the BoardView whenever its state changes.

export class Vertex {
    int id;
    Player *owner = nullptr;
    Assessment assessment = Assessment::None;

    std::vector<Vertex *> neighbours;
    std::vector<Edge   *> incidentEdges;
    std::vector<Tile   *> adjacentTiles;

    BoardView *observer = nullptr;

    void notifyObserver();

public:
    explicit Vertex(int id);

    int getId() const { return id; }

    // ownership / state
    Player    *getOwner() const { return owner; }
    Colour     getOwnerColour() const;
    Assessment currentAssessment() const { return assessment; }
    bool       isOccupied() const { return owner != nullptr; }

    // adjacency setup (called from GameBoard::initializeBoardGraph)
    void addNeighbour(Vertex *v);
    void addIncidentEdge(Edge *e);
    void addAdjacentTile(Tile *t);

    const std::vector<Vertex *> &getNeighbours()    const { return neighbours; }
    const std::vector<Edge   *> &getIncidentEdges() const { return incidentEdges; }
    const std::vector<Tile   *> &getAdjacentTiles() const { return adjacentTiles; }

    // Observer hookup
    void attach(BoardView *view);

    // --- game rules checks ---

    // Can 'colour' complete this vertex as an Assignment?
    // Currently enforces:
    //  - not already occupied
    //  - no neighbouring vertex is occupied
    //
    bool canBeCompletedBy(Colour colour) const;

    // Can 'colour' improve this vertex (Assignment->Midterm->Exam)?
    bool canBeImprovedBy(Colour colour) const;

    // Actions (GameBoard should call these only after rules + resources
    // are checked)
    //
    void complete(Player *p);

    // improve: Assignment->Midterm, Midterm->Exam
    void improve();
};
