export module Resources;

import <iosfwd>;

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
