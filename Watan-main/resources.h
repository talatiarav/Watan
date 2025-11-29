#ifndef RESOURCES_H
#define RESOURCES_H

#include <iosfwd>

enum class Resources {
    Caffeine,
    Lab,
    Lecture,
    Study,
    Tutorial,
    Netflix,
    None
};

std::ostream &operator<<(std::ostream &out, Resources resource);

#endif
