// loadeddice.cc
// Implementation of LoadedDice methods.
#include "loadeddice.h"

LoadedDice::LoadedDice() : presetValue(0) {
    // Initialize lastRoll to 0 as no roll has happened yet.
    lastRoll = 0;
}

// Set a specific dice roll value for the next roll.
void LoadedDice::setDie(int value) {
    // The valid range for the sum of two six-sided dice is 2 to 12.
    if (value < 2 || value > 12) {
        throw std::invalid_argument("LoadedDice value must be between 2 and 12.");
    }
    presetValue = value;
}

// Return the preset dice value as the roll result.
int LoadedDice::roll() {
    // Ensure a valid preset value has been set.
    if (presetValue < 2 || presetValue > 12) {
        throw std::logic_error("No valid dice value set for LoadedDice.");
    }
    // Use the preset value as the outcome.
    lastRoll = presetValue;
    return lastRoll;
}
