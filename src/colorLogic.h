#ifndef COLOR_LOGIC_H
#define COLOR_LOGIC_H

#include <set>
#include <vector>
#include <unordered_map>
#include <map>
#include "Point.h"
#include "Edge.h"
#include "AtomicEnclosure.h"
#include "Path.h"

using namespace std;

set<AtomicEnclosure> findAtomicEnclosures(const set<Point>& points, const set<Edge>& edges);
set<AtomicEnclosure> findAtomicEnclosures(const set<Edge>& edges);
void addEdgeAndCheckForNewEnclosures(Edge newEdge, unordered_map<Point, set<Edge>>& pointToEdgeMap, set<AtomicEnclosure>& allAtomicEnclosures);
set<AtomicEnclosure> convertSetOfPathsToAtomicEnclosure(set<Path> newEnclosures);
set<Path> findShortestPaths(const unordered_map<Point, set<Edge>>& pointToEdgeMap, Point startPoint);
unordered_map<Point, set<Edge>> createPointToEdgeMap(const set<Point>& points, const set<Edge>& edges);
void addEdgeToEdgeMap(const Edge edge, unordered_map<Point, set<Edge>>& pointToEdgeMap);
void addPointToEdgeMap(const Point point, unordered_map<Point, set<Edge>>& pointToEdgeMap);
bool edgeAlreadySearched(Edge edge, vector<Edge>& path);
bool edgesShareCommonPoint(const Edge& e1, const Edge& e2, const Edge& e3);
vector<Point> orderEnclosureBoundary(const AtomicEnclosure& enclosure);
vector<Point> fanTriangulate(const vector<Point>& boundary);

map<Edge, set<Edge>> createEdgeToEdgeMap(const set<Edge>& edges);
void addEdgeToEdgeToEdgeMap(const Edge edge, map<Edge, set<Edge>>& edgeToEdgeMap);
Path findShortestPath(const map<Edge, set<Edge>>& edgeToEdgeMap, Edge startEdge);

#endif // COLOR_LOGIC_H
