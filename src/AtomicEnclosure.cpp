#include <set>
#include "Point.h"
#include "Edge.h"

using namespace std;

struct AtomicEnclosure {
    set<Point> points;
    set<Edge> edges;

    AtomicEnclosure(const set<Point>& pts, const set<Edge>& eds) : points(pts), edges(eds) {}

    bool AtomicEnclosureHasEdge(Edge edge) const {
        return edges.count(edge) > 0;
    }
    
    bool sharesEdgeWith(const AtomicEnclosure& other) const {
        for (const Edge& edge : other.edges) {
            if (AtomicEnclosureHasEdge(edge)) {
                return true;
            }
        }
        return false;
    }

};