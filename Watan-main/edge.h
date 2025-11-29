export module Edge;

import <vector>;
import Colour;

class Player;
class Vertex;
class BoardView;

// Represents a goal edge on the board. Tracks its owner, endpoint vertices,
// and neighbouring edges. Used to check whether a player is allowed to achieve
// the goal, and notifies the BoardView whenever its state changes.


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

    void attach(BoardView *view);

    // Enforces:
    //  - edge is unowned
    //  - at least one endpoint vertex is owned by 'colour' OR
    //    at least one neighbouring edge is owned by 'colour'.
    bool canBeAchievedBy(Colour colour) const;

    // Action: mark as achieved by this player.
    void achieve(Player *p);
};
