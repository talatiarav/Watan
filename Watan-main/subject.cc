#include "subject.h"
#include "observer.h"
#include "vertex.h"
#include "edge.h"
#include "tile.h"
#include <algorithm>

Subject::~Subject() {}

void Subject::attach(Observer *observer) {
    observers.push_back(observer);
}

void Subject::detach(Observer *observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer),
                    observers.end());
}

void Subject::notifyObservers() {
    if (auto *v = dynamic_cast<Vertex*>(this)) {
        for (Observer *obs : observers) {
            obs->notify(v);
        }
    } else if (auto *e = dynamic_cast<Edge*>(this)) {
        for (Observer *obs : observers) {
            obs->notify(e);
        }
    } else if (auto *t = dynamic_cast<Tile*>(this)) {
        for (Observer *obs : observers) {
            obs->notify(t);
        }
    }
}
