#ifndef LINE_AND_POINT_LOGIC_H
#define LINE_AND_POINT_LOGIC_H

#include <unordered_map>
#include <optional>
#include <set>
#include "Point.h"
using namespace std;

// Declaration of the intersection calculation function
optional<Point> getIntersectionPoint(const set<Point>& line1, const set<Point>& line2);

optional<Point> closestPointOnLine(const Point& p, const Point& p1, const Point& p2);

void splitLineIntoSegments(const set<Point>& line, Point intersectionPoint, set<set<Point>>& allEdges);

void breakLineIntoSegments(const set<Point>& line, vector<Point> intersectionPoints, set<set<Point>>& allEdges);

bool isPointOnLineSegment(const Point& p, const set<Point>& line);

bool pointWithinSegments(const Point& p, const Point& p1, const Point& p2);

void getConstantsFromLine(const set<Point>& line1, const set<Point>& line2, float& a1, float& b1, float& c1, float& a2, float& b2, float& c2, float& determinant);

void getPointsFromLine(const set<Point>& line1, const set<Point>& line2, Point& p1, Point& p2, Point& p3, Point& p4);
#endif // LINE_AND_POINT_LOGIC_H