// dice.h
// Abstract Dice interface class for dice rolling strategies in the Students of Watan game.
#ifndef DICE_H
#define DICE_H

#include <stdexcept>

class Dice {
public:
    // Virtual destructor for safe polymorphic deletion
    virtual ~Dice() = default;

    // Roll the dice and return the outcome.
    // Implementations should update lastRoll accordingly.
    virtual int roll() = 0;

    // Optionally set a predetermined roll value (only meaningful for LoadedDice).
    // Default implementation: throw if not overridden by a subclass that supports it.
    virtual void setDie(int value) {
        // Base Dice (e.g., FairDice) does not support presetting a roll value.
        throw std::logic_error("Presetting dice value is not supported for this dice type.");
    }

protected:
    int lastRoll = 0;
};

#endif // DICE_H
