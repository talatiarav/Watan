#ifndef OBSERVER_H
#define OBSERVER_H

// Forward declarations of subject classes
class Vertex;
class Edge;
class Tile;

/**
 * Observer interface for classes that need to react to Subject updates.
 * BoardView implements this interface to display changes in the game board.
 */
class Observer {
public:
    virtual ~Observer() = default;
    virtual void notify(Vertex *vertex) = 0;
    virtual void notify(Edge *edge) = 0;
    virtual void notify(Tile *tile) = 0;
};

#endif
