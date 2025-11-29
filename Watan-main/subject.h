export module Subject;

import <vector>;

class Observer;

// Base class for observable objects in the game (e.g., vertices and edges).
// Allows observers like BoardView to attach and be notified when the subject changes,
// enabling automatic board updates without tight coupling.

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
