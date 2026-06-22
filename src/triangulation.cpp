#include <vector>
#include <set>
#include <unordered_map>
#include "triangulation.h"
#include "Point.h"
#include "Edge.h"
#include "AtomicEnclosure.h"
#include "lineAndPointLogic.h"

using namespace std;

vector<Point> orderEnclosureBoundary(const AtomicEnclosure& enclosure) {
    vector<Point> ordered;
    if (enclosure.edges.empty()) return ordered;

    unordered_map<Point, vector<Edge>> adjacency;
    for (const Edge& edge : enclosure.edges) {
        adjacency[edge.p1()].push_back(edge);
        adjacency[edge.p2()].push_back(edge);
    }

    set<Edge> usedEdges;
    Point start = *enclosure.points.begin();
    Point current = start;
    ordered.push_back(current);

    while (ordered.size() <= enclosure.edges.size()) {
        bool advanced = false;
        for (const Edge& edge : adjacency[current]) {
            if (usedEdges.count(edge)) continue;
            usedEdges.insert(edge);
            current = nearlyEqual(edge.p1(), current) ? edge.p2() : edge.p1();
            advanced = true;
            break;
        }
        if (!advanced || nearlyEqual(current, start)) break;
        ordered.push_back(current);
    }

    return ordered;
}

vector<Point> fanTriangulate(const vector<Point>& boundary) {
    vector<Point> triangles;
    for (size_t i = 1; i + 1 < boundary.size(); ++i) {
        triangles.push_back(boundary[0]);
        triangles.push_back(boundary[i]);
        triangles.push_back(boundary[i + 1]);
    }
    return triangles;
}

// Returns the z-component of the cross product (origin->a) x (origin->b).
// Positive = CCW turn, negative = CW turn, zero = collinear.
float crossProduct(const Point& origin, const Point& a, const Point& b) {
    return (a.x - origin.x) * (b.y - origin.y)
         - (a.y - origin.y) * (b.x - origin.x);
}

// A vertex v is convex (an "ear candidate") when the turn prev->v->next is CCW,
// meaning v does not dip inward. Boundary is expected in CCW order.
bool isConvexVertex(const Point& prev, const Point& v, const Point& next) {
    return crossProduct(prev, v, next) > 0.0f;
}

// Returns true if point p lies strictly inside triangle (a, b, c).
// Uses the same-sign cross product test: p must be on the left side of
// every directed edge a->b, b->c, c->a.
bool pointInTriangle(const Point& p, const Point& a, const Point& b, const Point& c) {
    float d1 = crossProduct(a, b, p);
    float d2 = crossProduct(b, c, p);
    float d3 = crossProduct(c, a, p);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // Point is inside only if all cross products share the same sign (all pos or all neg).
    // Returning false for points exactly on the edge (zero) avoids clipping shared vertices.
    return !(hasNeg && hasPos) && (d1 != 0 && d2 != 0 && d3 != 0);
}

// A vertex at [index] is an ear if:
// 1. It is a convex vertex (turn CCW relative to its neighbors).
// 2. No other polygon vertex lies strictly inside the triangle it forms.
bool isEar(int index, const vector<Point>& vertices) {
    int n = (int)vertices.size();
    int prev = (index - 1 + n) % n;
    int next = (index + 1) % n;

    if (!isConvexVertex(vertices[prev], vertices[index], vertices[next]))
        return false;

    for (int i = 0; i < n; i++) {
        if (i == prev || i == index || i == next) continue;
        if (pointInTriangle(vertices[i], vertices[prev], vertices[index], vertices[next]))
            return false;
    }

    return true;
}

// Ear clipping: iteratively removes ears until only one triangle remains.
// Works correctly on simple polygons (convex or concave). O(n²).
vector<Point> earClipTriangulate(const vector<Point>& boundary) {
    vector<Point> triangles;
    vector<Point> vertices = boundary;

    // Ensure vertices are in CCW order. orderEnclosureBoundary walks edges
    // without a guaranteed winding direction, so we compute the signed area
    // and reverse if the result is CW (negative). isConvexVertex relies on
    // positive cross products which only hold for CCW-wound polygons.
    double signedArea = 0.0;
    int sz = (int)vertices.size();
    for (int i = 0; i < sz; i++) {
        const Point& a = vertices[i];
        const Point& b = vertices[(i + 1) % sz];
        signedArea += (a.x * b.y - b.x * a.y);
    }
    if (signedArea < 0.0)
        reverse(vertices.begin(), vertices.end());

    while (vertices.size() > 3) {
        bool earFound = false;
        int n = (int)vertices.size();

        for (int i = 0; i < n; i++) {
            if (isEar(i, vertices)) {
                int prev = (i - 1 + n) % n;
                int next = (i + 1) % n;

                // Emit the ear triangle
                triangles.push_back(vertices[prev]);
                triangles.push_back(vertices[i]);
                triangles.push_back(vertices[next]);

                // Remove the ear vertex and continue
                vertices.erase(vertices.begin() + i);
                earFound = true;
                break;
            }
        }

        // Guard: if no ear found (degenerate polygon), stop to avoid infinite loop
        if (!earFound) break;
    }

    // Emit the last remaining triangle
    if (vertices.size() == 3) {
        triangles.push_back(vertices[0]);
        triangles.push_back(vertices[1]);
        triangles.push_back(vertices[2]);
    }

    return triangles;
}
