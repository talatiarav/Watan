#ifndef VERTEX_H
#define VERTEX_H

#include <vector>
#include "assessment.h"
#include "colour.h"

class Player;
class Edge;
class Tile;
class BoardView;

/**
 * Vertex
 *
 * Represents a single board intersection / course criterion.
 * Roughly the new version of the old Criterion class.
 *
 * State:
 *  - id (0–53)
 *  - owner (Player*, nullptr if unowned)
 *  - assessment (None / Assignment / Midterm / Exam)
 *  - adjacency: neighbouring vertices, incident edges, adjacent tiles
 *  - a single BoardView observer (for ASCII rendering)
 */
class Vertex {
    int id;
    Player *owner = nullptr;
    Assessment assessment = Assessment::None;

    std::vector<Vertex *> neighbours;
    std::vector<Edge   *> incidentEdges;
    std::vector<Tile   *> adjacentTiles;

    BoardView *observer = nullptr;

    void notifyObserver() const;

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
    // NOTE: The spec also requires adjacency to an owned goal except
    // during setup. That can be layered on later (either here or by
    // having GameBoard/GameController call a different API during setup).
    bool canBeCompletedBy(Colour colour) const;

    // Can 'colour' improve this vertex (Assignment->Midterm->Exam)?
    bool canBeImprovedBy(Colour colour) const;

    // Actions (GameBoard should call these only after rules + resources
    // are checked)
    //
    // complete: set owner and move to Assignment
    void complete(Player *p);

    // improve: Assignment->Midterm, Midterm->Exam
    void improve();
};

#endif // VERTEX_H
