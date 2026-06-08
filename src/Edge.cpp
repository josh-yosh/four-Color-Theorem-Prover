#include <set>
#include "Edge.h"
#include "Point.h"

using namespace std;

struct Edge {
    set<Point> endpoints;

    void addEndpoint(const Point& p) {
        if (endpoints.size() < 2) {
            endpoints.insert(p);
        } else {
            throw runtime_error("An edge can only have two endpoints.");
        }
    }

    bool operator<(const Edge& other) const{
        bool equal = other == *this;
        if (equal) return false; // they are equal, so not less than

        Point a1 = *endpoints.begin();
        Point a2 = *next(endpoints.begin());
        Point b1 = *other.endpoints.begin();
        Point b2 = *next(other.endpoints.begin());

        // Sort points to ensure consistent ordering for comparison
        if (a1 < a2) swap(a1, a2);
        if (b1 < b2) swap(b1, b2);
        if (a1 < b1) return true;
        if (b1 < a1) return false;
        return a2 < b2;
    }
    bool operator==(const Edge& other) const{
        Point a1 = *endpoints.begin();
        Point a2 = *next(endpoints.begin());
        Point b1 = *other.endpoints.begin();
        Point b2 = *next(other.endpoints.begin());

        // Sort points to ensure consistent ordering for comparison
        if (a1 < a2) swap(a1, a2);
        if (b1 < b2) swap(b1, b2);
        return nearlyEqual(a1, b1) && nearlyEqual(a2, b2) || nearlyEqual(a1, b2) && nearlyEqual(a2, b1);
    }
};