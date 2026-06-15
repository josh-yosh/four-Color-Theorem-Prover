#include "Path.h"

bool Path::operator<(const Path& other) const {
    return edges < other.edges;
}

bool Path::operator==(const Path& other) const {
    return edges == other.edges;
}

bool Path::isEmptyPath() const {
    return edges.size() == 0;
}