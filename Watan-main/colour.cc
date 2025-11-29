module Colour;

import <iostream>;
import <cstdlib>;

// Defines the Colour enum and how colours are printed.
// Supports optional ANSI colour output for the board, but player names
// always print in plain text. Also provides an operator<< for easy display.


// Global flag for colored output (only affects board display in boardview.cc)
static bool useColors = false;

void Colour_enableColors(bool enable) {
    useColors = enable;
}

std::ostream &operator<<(std::ostream &out, Colour colour) {
    switch (colour) {
        case Colour::Blue:   return out << "Blue";
        case Colour::Red:    return out << "Red";
        case Colour::Orange: return out << "Orange";
        case Colour::Yellow: return out << "Yellow";
        case Colour::Bank:   return out << "Bank";
        case Colour::None:   return out << "None";
    }
    return out;
}
