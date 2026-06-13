#include <set>
#include "Point.h"
#include "Edge.h"
#include "AtomicEnclosure.h"

using namespace std;

bool AtomicEnclosure::atomicEnclosureHasEdge(Edge edge) const {
    return edges.count(edge) > 0;
}

bool AtomicEnclosure::sharesEdgeWith(const AtomicEnclosure& other) const {
    for (const Edge& edge : other.edges) {
        if (atomicEnclosureHasEdge(edge)) {
            return true;
        }
    }
    return false;
}

bool AtomicEnclosure::operator==(const AtomicEnclosure& other) const {
    return points == other.points && edges == other.edges;
}

bool AtomicEnclosure::operator<(const AtomicEnclosure& other) const {
    if (points != other.points) return points < other.points;
    return edges < other.edges;
}
