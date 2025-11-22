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

std::ostream &operator<<(std::ostream &out, Colour colour);

#endif
