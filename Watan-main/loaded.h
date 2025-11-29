export module Loaded;

// Implements loaded dice, allowing the player to manually set the roll value.
// setDie() stores the chosen number, and roll() simply returns it.
// Used when players switch to loaded dice during their turn.

import Dice;

export class Loaded : public Dice {
    int value;
public:
    Loaded(); //ctor
    void setDie(int num);
    int roll();
};
