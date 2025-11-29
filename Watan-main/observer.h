export module Observer;


// Defines the Observer interface used by BoardView and other display components.
// Subjects (like vertices and edges) notify observers when their state changes,
// allowing the board to update its ASCII display without tight coupling.

// Forward declarations of subject classes
class Vertex;
class Edge;
class Tile;


export class Observer {
public:
    virtual ~Observer() = default;
    virtual void notify(Vertex *vertex) = 0;
    virtual void notify(Edge *edge) = 0;
    virtual void notify(Tile *tile) = 0;
};
