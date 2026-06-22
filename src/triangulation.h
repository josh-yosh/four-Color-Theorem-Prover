#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <vector>
#include "Point.h"
#include "AtomicEnclosure.h"

using namespace std;

vector<Point> orderEnclosureBoundary(const AtomicEnclosure& enclosure);
vector<Point> fanTriangulate(const vector<Point>& boundary);

// Ear clipping helpers
float crossProduct(const Point& origin, const Point& a, const Point& b);
bool isConvexVertex(const Point& prev, const Point& v, const Point& next);
bool pointInTriangle(const Point& p, const Point& a, const Point& b, const Point& c);
bool isEar(int index, const vector<Point>& vertices);

// Ear clipping triangulation — works on simple (non-convex) polygons
vector<Point> earClipTriangulate(const vector<Point>& boundary);

#endif // TRIANGULATION_H
