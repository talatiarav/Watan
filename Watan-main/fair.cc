module Fair;

import <stdlib.h>;
import <iostream>;

// Implements fair dice that generate random rolls from 2–12.
// setDie() is unused here, since fair dice cannot be forced,
// and roll() returns the sum of two random 1–6 values.

using namespace std;

Fair::Fair() {
  dice1 = 0;
  dice2 = 0;
}

void Fair::setDie(int num) {}

// roll dice
int Fair::roll() {
  dice1 = rand() % 6 + 1;
  dice2 = rand() % 6 + 1;
  std::cout << "Dice rolled: " << dice1 + dice2 << std::endl;
  return dice1 + dice2;
}
