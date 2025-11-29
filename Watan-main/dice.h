#ifndef _DICE_H_
#define _DICE_H_
#include <string>

class Dice {
	public:
		virtual ~Dice() = default;

		virtual void setDie(int num) = 0;
		virtual int roll() = 0;
};

#endif
