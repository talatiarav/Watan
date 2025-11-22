#include "colour.h"
#include <iostream>

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

