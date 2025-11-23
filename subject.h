#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>
#include "state.h"

class Observer;

/**
 * Subject base class for observable board elements (Vertex, Edge, Tile).
 * Maintains a list of observers (e.g., BoardView) and notifies them on changes.
 */
class Subject {
private:
    std::vector<Observer*> observers;
protected:
    void notifyObservers();
public:
    virtual ~Subject() = 0;
    void attach(Observer *observer);
    void detach(Observer *observer);
    virtual State getState() const = 0;
};

#endif
