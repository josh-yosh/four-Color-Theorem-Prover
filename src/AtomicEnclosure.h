#ifndef ATOMICENCLOSURE_H
#define ATOMICENCLOSURE_H

#include <set>
#include "Point.h"
#include "Edge.h"
#include "Path.h"

using namespace std;

struct AtomicEnclosure {
    set<Point> points;
    set<Edge> edges;

    AtomicEnclosure(const set<Point>& pts, const set<Edge>& eds) : points(pts), edges(eds) {}
    AtomicEnclosure(const Path& path);

    bool sharesEdgeWith(const AtomicEnclosure& other) const;
    bool atomicEnclosureHasEdge(Edge edge) const;

    bool operator<(const AtomicEnclosure& other) const;
    bool operator==(const AtomicEnclosure& other) const;
};

#endif 