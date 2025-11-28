#include "dice.h"
#include "fair.h"
#include "loaded.h"
#include <string>
using namespace std;

Dice *Dice::make_dice(string choice) {

	if (choice == "loaded") {
		return new Loaded;
	} else {
		return new Fair;
	}
}
