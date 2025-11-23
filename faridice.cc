// fairdice.cc
// Implementation of FairDice methods.
#include "fairdice.h"

// Initialize the FairDice random engine with the given seed.
FairDice::FairDice(unsigned int seed) : engine(seed) {}

// Roll two random values in [1,6] and return their sum.
int FairDice::roll() {
    // Define a uniform distribution for values 1 through 6.
    std::uniform_int_distribution<int> dist(1, 6);
    // Roll two dice using the distribution and random engine.
    int die1 = dist(engine);
    int die2 = dist(engine);
    lastRoll = die1 + die2;
    return lastRoll;
}
