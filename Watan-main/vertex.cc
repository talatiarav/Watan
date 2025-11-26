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

bool Vertex::canBeCompletedBy(Colour /*colour*/) const {
    if (isOccupied()) return false;

    // No adjacent criterion may be completed (spec 3.1).
    for (Vertex *v : neighbours) {
        if (v && v->isOccupied()) {
            return false;
        }
    }

    // NOTE: The “must be adjacent to your goal unless at setup” rule
    // can be added by checking incidentEdges here against the player's
    // colour once we pass a Player* instead of just Colour, or by providing
    // a separate API for setup placements.
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
