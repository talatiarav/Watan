module Edge;

import Player;
import Vertex;
import BoardView;
import Colour;

// Represents a goal edge on the board. Tracks its owner, endpoint vertices,
// and neighbouring edges. Used to check whether a player is allowed to achieve
// the goal, and notifies the BoardView whenever its state changes.

Edge::Edge(int id)
    : id{id} {}

Colour Edge::getOwnerColour() const {
    return owner ? owner->getColour() : Colour::None;
}

void Edge::setEndpoints(Vertex *v1, Vertex *v2) {
    endpoints[0] = v1;
    endpoints[1] = v2;
}

void Edge::addNeighbour(Edge *e) {
    if (!e) return;
    neighbours.emplace_back(e);
}

void Edge::attach(BoardView *view) {
    observer = view;
}

void Edge::notifyObserver() {
    if (observer) {
        observer->notify(this);
    }
}

bool Edge::canBeAchievedBy(Colour colour) const {
    if (owner) return false; // already taken

    // Endpoint criterion completed by this player?
    for (int i = 0; i < 2; ++i) {
        Vertex *v = endpoints[i];
        if (v && v->isOccupied() && v->getOwnerColour() == colour) {
            return true;
        }
    }

    // Or an adjacent goal already achieved by this player?
    for (Edge *e : neighbours) {
        if (e && e->getOwnerColour() == colour) {
            return true;
        }
    }

    return false;
}

void Edge::achieve(Player *p) {
    if (!p) return;
    owner = p;
    notifyObserver();
}
