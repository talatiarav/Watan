export module Resources;

import <iosfwd>;

// Implements helper functions for working with resource types.
// Provides string conversions, printing, and any utility logic
// used throughout the game for handling resource names and values.


export enum class Resources {
    Caffeine,
    Lab,
    Lecture,
    Study,
    Tutorial,
    Netflix,
    None
};

export std::ostream &operator<<(std::ostream &out, Resources resource);
