export module Dice;

import <string>;

export class Dice {
public:
    virtual ~Dice() = default;

    virtual void setDie(int num) = 0;
    virtual int roll() = 0;
};
