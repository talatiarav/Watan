// loadeddice.h
// LoadedDice class – returns a preset dice roll value (for testing or cheating).
#ifndef LOADEDDICE_H
#define LOADEDDICE_H

#include "dice.h"

// Exception class for invalid dice value inputs.
#include <stdexcept>

class LoadedDice : public Dice {
public:
    // Construct a LoadedDice. Initially no value is set.
    LoadedDice();

    // Set the next roll value. Must be between 2 and 12 (inclusive).
    // Throws std::invalid_argument if value is out of range.
    void setDie(int value) override;

    // Return the preset value as the dice roll result.
    // If no value has been set since last roll, uses the last set value.
    // Updates lastRoll with the result.
    int roll() override;

private:
    int presetValue;
};

#endif // LOADEDDICE_H
