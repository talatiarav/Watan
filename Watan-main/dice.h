export module Dice;


// Abstract base class for all dice types in the game.
// Provides a common interface for setting a chosen roll (loaded dice)
// and generating a roll (fair or loaded). Used so players can swap dice at runtime.


import <string>;

export class Dice {
public:
    virtual ~Dice() = default;

    virtual void setDie(int num) = 0;
    virtual int roll() = 0;
};
