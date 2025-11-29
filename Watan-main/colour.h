export module Colour;

import <iosfwd>;

// Defines the Colour enum and how colours are printed.
// Supports optional ANSI colour output for the board, but player names
// always print in plain text. Also provides an operator<< for easy display.


export enum class Colour {
    Blue,
    Red,
    Orange,
    Yellow,
    Bank,
    None
};

export void Colour_enableColors(bool enable);

export std::ostream &operator<<(std::ostream &out, Colour colour);
