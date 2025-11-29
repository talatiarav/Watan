#include "vertex.h"

#include "player.h"
#include "edge.h"
#include "tile.h"
#include "boardview.h"

Vertex::Vertex(int id)
    : id{id} {}

Colour Vertex::getOwnerColour() const {
    return owner ? owner->getColour() : Colour::None;
}

void Vertex::addNeighbour(Vertex *v) {
    if (!v) return;
    neighbours.emplace_back(v);
}

void Vertex::addIncidentEdge(Edge *e) {
    if (!e) return;
    incidentEdges.emplace_back(e);
}

void Vertex::addAdjacentTile(Tile *t) {
    if (!t) return;
    adjacentTiles.emplace_back(t);
}

void Vertex::attach(BoardView *view) {
    observer = view;
}

void Vertex::notifyObserver() {
    if (observer) {
        observer->notify(this);
    }
}

bool Vertex::canBeCompletedBy(Colour colour) const {
    if (isOccupied()) return false;

    // For each neighbouring vertex:
    for (Vertex *v : neighbours) {
        if (!v || !v->isOccupied()) continue;

        Colour neighbourColour = v->getOwnerColour();

        // If the neighbour belongs to another player, it always blocks.
        if (neighbourColour != colour) {
            return false;
        }

        // If the neighbour is the same colour, only allow it to be “ignored”
        // if there is a road (goal) owned by this colour directly between the two.
        bool connectedByOwnedRoad = false;

        for (Edge *eThis : incidentEdges) {
            if (!eThis || eThis->getOwnerColour() != colour) continue;

            // Same-class methods can access private members of other Vertex objects,
            // so we can inspect v->incidentEdges here.
            for (Edge *eNeighbour : v->incidentEdges) {
                if (eNeighbour == eThis) {
                    connectedByOwnedRoad = true;
                    break;
                }
            }

            if (connectedByOwnedRoad) break;
        }

        // If we have a same-colour neighbour but no owned road connecting us,
        // it still blocks (same as before).
        if (!connectedByOwnedRoad) {
            return false;
        }
    }

    // All checks passed: this vertex can be completed.
    return true;
}

bool Vertex::canBeImprovedBy(Colour colour) const {
    if (!owner) return false;
    if (owner->getColour() != colour) return false;

    // Only Assignment or Midterm can be improved.
    return assessment == Assessment::Assignment ||
           assessment == Assessment::Midterm;
}

void Vertex::complete(Player *p) {
    if (!p) return;
    owner      = p;
    assessment = Assessment::Assignment;
    notifyObserver();
}

void Vertex::improve() {
    if (!owner) return;

    if (assessment == Assessment::Assignment) {
        assessment = Assessment::Midterm;
    } else if (assessment == Assessment::Midterm) {
        assessment = Assessment::Exam;
    } else {
        // Nothing to do (already Exam or None).
        return;
    }
    notifyObserver();
}
