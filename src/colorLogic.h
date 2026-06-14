#ifndef COLOR_LOGIC_H
#define COLOR_LOGIC_H

#include <set>
#include <vector>
#include <unordered_map>
#include "Point.h"
#include "Edge.h"
#include "AtomicEnclosure.h"

using namespace std;

set<AtomicEnclosure> findAtomicEnclosures(const set<Point>& points, const set<Edge>& edges);
void addEdgeAndCheckForNewEnclosures(Edge newEdge, unordered_map<Point, set<Edge>>& pointToEdgeMap, set<AtomicEnclosure>& allAtomicEnclosures);
set<AtomicEnclosure> convertSetOfPathsToAtomicEnclosure(set<vector<Edge>> newEnclosures);
set<vector<Edge>> findShortestPaths(const unordered_map<Point, set<Edge>>& pointToEdgeMap, Point startPoint);
unordered_map<Point, set<Edge>> createPointToEdgeMap(const set<Point>& points, const set<Edge>& edges);
void addEdgeToEdgeMap(const Edge edge, unordered_map<Point, set<Edge>>& pointToEdgeMap);
void addPointToEdgeMap(const Point point, unordered_map<Point, set<Edge>>& pointToEdgeMap);
bool edgeAlreadySearched(Edge edge, vector<Edge>& path);
vector<Point> orderEnclosureBoundary(const AtomicEnclosure& enclosure);
vector<Point> fanTriangulate(const vector<Point>& boundary);

#endif // COLOR_LOGIC_H
