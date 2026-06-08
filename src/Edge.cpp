#include <set>
#include <stdexcept>
#include "Edge.h"
#include "Point.h"

using namespace std;

void Edge::addEndpoint(const Point& p) {
    if (endpoints.size() < 2) {
        endpoints.insert(p);
    } else {
        throw runtime_error("An edge can only have two endpoints.");
    }
}

bool Edge::operator<(const Edge& other) const {
    if (*this == other) return false;

    Point a1 = p1(), a2 = p2();
    Point b1 = other.p1(), b2 = other.p2();

    if (a1 < b1) return true;
    if (b1 < a1) return false;
    return a2 < b2;
}

bool Edge::operator==(const Edge& other) const {
    Point a1 = p1(), a2 = p2();
    Point b1 = other.p1(), b2 = other.p2();

    return (nearlyEqual(a1, b1) && nearlyEqual(a2, b2)) ||
           (nearlyEqual(a1, b2) && nearlyEqual(a2, b1));
}
