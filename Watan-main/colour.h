#ifndef COLOUR_H
#define COLOUR_H

#include <iosfwd>

enum class Colour {
    Blue,
    Red,
    Orange,
    Yellow,
    Bank,
    None
};

// Enable/disable colored output for player names
void Colour_enableColors(bool enable);

std::ostream &operator<<(std::ostream &out, Colour colour);

#endif
