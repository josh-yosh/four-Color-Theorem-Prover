#ifndef EDGE_H
#define EDGE_H

#include <set>
#include <stdexcept>
#include "Point.h"

using namespace std;

struct Edge {
    set<Point> endpoints;

    Edge() = default;
    Edge(const Point& a, const Point& b) {
        endpoints.insert(a);
        endpoints.insert(b);
    }

    void addEndpoint(const Point& p);
    bool isComplete() const { return endpoints.size() == 2; }

    Point p1() const { return *endpoints.begin(); }
    Point p2() const { return *next(endpoints.begin()); }

    bool operator<(const Edge& other) const;
    bool operator==(const Edge& other) const;
};

#endif // EDGE_H
