#ifndef EDGE_H
#define EDGE_H

#include <set>
#include "Point.h"

using namespace std;

struct Edge {
    set<Point> endpoints;

    void addEndpoint(const Point& p);
    bool operator<(const Edge& other) const;   // so it can live in a std::set
    bool operator==(const Edge& other) const;
};

#endif 