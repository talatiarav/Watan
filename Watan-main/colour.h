export module Colour;

import <iosfwd>;

export enum class Colour {
    Blue,
    Red,
    Orange,
    Yellow,
    Bank,
    None
};

// Enable/disable colored output for player names
export void Colour_enableColors(bool enable);

export std::ostream &operator<<(std::ostream &out, Colour colour);
