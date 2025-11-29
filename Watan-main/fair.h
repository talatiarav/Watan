export module Fair;

import Dice;

export class Fair : public Dice {
    int dice1;
    int dice2;

public:
    Fair(); // ctor
    void setDie(int num) override;
    int roll() override;
};
