module Resources;

import <iostream>;

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
