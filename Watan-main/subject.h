export module Subject;

import <vector>;

class Observer;

/**
 * Subject base class for observable board elements (Vertex, Edge, Tile).
 * Maintains a list of observers (e.g., BoardView) and notifies them on changes.
 */
export class Subject {
private:
    std::vector<Observer*> observers;
protected:
    void notifyObservers();
public:
    virtual ~Subject() = 0;
    void attach(Observer *observer);
    void detach(Observer *observer);
};
