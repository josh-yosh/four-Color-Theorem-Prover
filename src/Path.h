#ifndef PATH_H
#define PATH_H

#include <set>
#include <vector>
#include "Edge.h"

using namespace std;

// A Path represents a loop of edges. Two paths are considered equal if they
// consist of the same set of edges, regardless of the order or direction
// they were traversed in.
struct Path {
    set<Edge> edges;

    Path() = default;
    Path(const vector<Edge>& path) : edges(path.begin(), path.end()) {}
    Path(const set<Edge>& edgeSet) : edges(edgeSet) {}

    bool operator<(const Path& other) const;
    bool operator==(const Path& other) const;

    bool isEmptyPath() const;
};

#endif // PATH_H
