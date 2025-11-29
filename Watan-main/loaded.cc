module Loaded;

import <iostream>;
import Loaded;

// Implements loaded dice, allowing the player to manually set the roll value.
// setDie() stores the chosen number, and roll() simply returns it.
// Used when players switch to loaded dice during their turn.


using std::endl;

Loaded::Loaded() {}

void Loaded::setDie(int num) {
    value = num;
}

int Loaded::roll() {
    std::cout << "Dice rolled: " << value << endl;
    return value;
}
