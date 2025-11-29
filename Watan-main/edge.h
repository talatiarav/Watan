export module Edge;

import <vector>;
import Colour;

// Forward declarations to avoid circular dependencies at interface level
class Player;
class Vertex;
class BoardView;

/**
 * Edge
 *
 * Represents a single goal edge between two vertices.
 * Roughly the new version of old Goal.
 *
 * State:
 *  - id (0–71)
 *  - owner (Player*, nullptr if unowned)
 *  - endpoints (two Vertex*)
 *  - neighbouring edges (for “adjacent goal” rule)
 *  - a BoardView observer for display
 */
export class Edge {
    int id;
    Player *owner = nullptr;

    Vertex *endpoints[2] = {nullptr, nullptr};
    std::vector<Edge *> neighbours;

    BoardView *observer = nullptr;

    void notifyObserver();

public:
    explicit Edge(int id);

    int getId() const { return id; }

    Player *getOwner() const { return owner; }
    Colour  getOwnerColour() const;

    void setEndpoints(Vertex *v1, Vertex *v2);
    Vertex *getEndpoint(int i) const { return endpoints[i]; }

    void addNeighbour(Edge *e);
    const std::vector<Edge *> &getNeighbours() const { return neighbours; }

    // Observer hookup
    void attach(BoardView *view);

    // Rule check: can 'colour' achieve this goal?
    //
    // Enforces:
    //  - edge is unowned
    //  - at least one endpoint vertex is owned by 'colour' OR
    //    at least one neighbouring edge is owned by 'colour'.
    bool canBeAchievedBy(Colour colour) const;

    // Action: mark as achieved by this player.
    // (GameBoard handles resource checks & Player bookkeeping.)
    void achieve(Player *p);
};
