module Colour;

import <iostream>;
import <cstdlib>;

// Global flag for colored output (only affects board display in boardview.cc)
static bool useColors = false;

void Colour_enableColors(bool enable) {
    useColors = enable;
}

std::ostream &operator<<(std::ostream &out, Colour colour) {
    // Always use plain text for player names (colors only on board)
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
