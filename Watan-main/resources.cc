module Resources;

import <iostream>;

// Implements helper functions for working with resource types.
// Provides string conversions, printing, and any utility logic
// used throughout the game for handling resource names and values.


std::ostream &operator<<(std::ostream &out, Resources resource) {
    switch (resource) {
        case Resources::Caffeine: return out << "Caffeine";
        case Resources::Lab:      return out << "Lab";
        case Resources::Lecture:  return out << "Lecture";
        case Resources::Study:    return out << "Study";
        case Resources::Tutorial: return out << "Tutorial";
        case Resources::Netflix:  return out << "Netflix";
        case Resources::None:     return out << "None";
    }
    return out;
}
