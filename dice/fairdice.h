// fairdice.h
// FairDice class – rolls two fair six-sided dice and returns their sum.
#ifndef FAIRDICE_H
#define FAIRDICE_H

#include "dice.h"
#include <random>

class FairDice : public Dice {
public:
    // Construct a FairDice and seed its random number generator.
    explicit FairDice(unsigned int seed);

    // Roll two fair dice (each 1-6) and return their sum.
    // Updates lastRoll with the result.
    int roll() override;

private:
    // Mersenne Twister random number engine for generating dice rolls.
    std::mt19937 engine;
};

#endif // FAIRDICE_H
