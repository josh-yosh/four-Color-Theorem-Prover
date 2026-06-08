#ifndef ATOMICENCLOSURE_H
#define ATOMICENCLOSURE_H

#include <set>
#include "Point.h"
#include "Edge.h"

using namespace std;

struct AtomicEnclosure {
    set<Point> points;
    set<Edge> edges;

    AtomicEnclosure(const set<Point>& pts, const set<Edge>& eds) : points(pts), edges(eds) {}
    
    double area() const;
    bool contains(const Point& p) const;
    bool sharesEdgeWith(const AtomicEnclosure& other) const;

};

#endif 