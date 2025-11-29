export module Loaded;

import Dice;

export class Loaded : public Dice {
    int value;
public:
    Loaded(); //ctor
    void setDie(int num);
    int roll();
};
