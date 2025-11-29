module Loaded;

import <iostream>;
import Loaded;

using std::endl;

Loaded::Loaded() {}

void Loaded::setDie(int num) {
    value = num;
}

int Loaded::roll() {
    std::cout << "Dice rolled: " << value << endl;
    return value;
}
