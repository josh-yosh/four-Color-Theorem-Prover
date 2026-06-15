#include <set>
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include "colorLogic.h"
#include "AtomicEnclosure.h"
#include "Path.h"


set<AtomicEnclosure> findAtomicEnclosures(const set<Point>& points, const set<Edge>& edges) {
    unordered_map<Point, set<Edge>> pointToEdgeMap = createPointToEdgeMap(points, edges);
    set<AtomicEnclosure> allAtomicEnclosures;

    for(Point point : points){
        set<Path> shortestPaths = findShortestPaths(pointToEdgeMap, point);

        for(const Path& path : shortestPaths){
            set<Point> pointsInPath;

            for(const Edge& edge : path.edges){
                pointsInPath.insert(edge.endpoints.begin(), edge.endpoints.end());
            }

            allAtomicEnclosures.insert(AtomicEnclosure(pointsInPath, path.edges));
        }
    }

    return allAtomicEnclosures;
}

set<AtomicEnclosure> findAtomicEnclosures(const set<Edge>& edges) {
    map<Edge, set<Edge>> edgeToEdgeMap = createEdgeToEdgeMap(edges);
    set<AtomicEnclosure> allAtomicEnclosures;

    for(const Edge& startEdge : edges){
        Path shortestPath = findShortestPath(edgeToEdgeMap, startEdge);
        if(!shortestPath.isEmptyPath()) { allAtomicEnclosures.insert(AtomicEnclosure(shortestPath)); }
    }

    
    return allAtomicEnclosures;
}

void addEdgeAndCheckForNewEnclosures(Edge newEdge, unordered_map<Point, set<Edge>>& pointToEdgeMap, set<AtomicEnclosure>& allAtomicEnclosures){
    addEdgeToEdgeMap(newEdge, pointToEdgeMap);

    set<Path> newEnclosures;
    for(Point endpoint : newEdge.endpoints){
        set<Path> paths = findShortestPaths(pointToEdgeMap, endpoint);
        newEnclosures.insert(paths.begin(), paths.end());
    }
    set<AtomicEnclosure> newAtomicEnclosures = convertSetOfPathsToAtomicEnclosure(newEnclosures);
    allAtomicEnclosures.insert(newAtomicEnclosures.begin(), newAtomicEnclosures.end());
}

set<AtomicEnclosure> convertSetOfPathsToAtomicEnclosure(set<Path> newEnclosures){
    set<AtomicEnclosure> allAtomicEnclosures;
    for(const Path& path : newEnclosures){
        set<Point> pointsInPath;
        for(const Edge& edge : path.edges){
            pointsInPath.insert(edge.endpoints.begin(), edge.endpoints.end());
        }
        allAtomicEnclosures.insert(AtomicEnclosure(pointsInPath, path.edges));
    }

    return allAtomicEnclosures;
}

set<Path> findShortestPaths(const unordered_map<Point, set<Edge>>& pointToEdgeMap, Point startPoint) {
    queue<vector<Edge>> queue;
    set<Path> paths;
    int shortestPathLength = 99;
    int currentPathLength = 1;

    for(Edge edge : pointToEdgeMap.at(startPoint)){
        queue.push({edge});
    }

    while(!queue.empty() && currentPathLength <= shortestPathLength){
        currentPathLength += 1;
        vector<Edge> currentPath = queue.front();
        Edge last = currentPath.back();
        for(Point endpoint : last.endpoints){
            if(endpoint == startPoint && currentPath.size() > 2){
                paths.insert(Path(currentPath));
                shortestPathLength = currentPathLength;
            } else {
                set<Edge> allEdgesFromLastPoint = pointToEdgeMap.at(endpoint);
                for(Edge edge : allEdgesFromLastPoint){
                    if(!edgeAlreadySearched(edge, currentPath)){
                        vector<Edge> newPath = currentPath;
                        newPath.push_back(edge);
                        queue.push(newPath);
                    }
                }
            }
        }
        queue.pop();
    }
    return paths;
}

unordered_map<Point, set<Edge>> createPointToEdgeMap(const set<Point>& points, const set<Edge>& edges){
    unordered_map<Point, set<Edge>> pointToEdgeMap;
    for (const Point& point : points){
        set<Edge> edgesContainingPoint = {};
        for(const Edge& edge : edges) {
            if(edge.hasEndpoint(point)){
                edgesContainingPoint.emplace(edge);
            }
        }
        pointToEdgeMap.insert({point, edgesContainingPoint});
    }
    return pointToEdgeMap;
}

void addEdgeToEdgeMap(const Edge edge, unordered_map<Point, set<Edge>>& pointToEdgeMap){
    Point p1 = edge.p1();
    Point p2 = edge.p2();

    pointToEdgeMap[p1].insert(edge);
    pointToEdgeMap[p2].insert(edge);
}

void addPointToEdgeMap(const Point point, unordered_map<Point, set<Edge>>& pointToEdgeMap){
    pointToEdgeMap[point];
}

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

bool edgesShareCommonPoint(const Edge& e1, const Edge& e2, const Edge& e3){
    for(const Point& point : e1.endpoints){
        if(e2.hasEndpoint(point) && e3.hasEndpoint(point)){
            return true;
        }
    }
    return false;
}

bool edgeAlreadySearched(Edge edge, vector<Edge>& path){
    for(Edge exploredPath : path){
        if (exploredPath == edge) return true;
    }
    return false;
}

map<Edge, set<Edge>> createEdgeToEdgeMap(const set<Edge>& edges){
    map<Edge, set<Edge>> edgeToEdgeMap;
    for(const Edge& edge : edges){
        set<Edge> edgesSharingPoint = {};
        for(const Edge& otherEdge : edges){
            if(otherEdge == edge) continue;
            if(otherEdge.hasEndpoint(edge.p1()) || otherEdge.hasEndpoint(edge.p2())){
                edgesSharingPoint.insert(otherEdge);
            }
        }
        edgeToEdgeMap.insert({edge, edgesSharingPoint});
    }
    return edgeToEdgeMap;
}

void addEdgeToEdgeToEdgeMap(const Edge edge, map<Edge, set<Edge>>& edgeToEdgeMap){
    set<Edge> edgesSharingPoint = {};
    for(auto& [otherEdge, sharedEdges] : edgeToEdgeMap){
        if(otherEdge.hasEndpoint(edge.p1()) || otherEdge.hasEndpoint(edge.p2())){
            edgesSharingPoint.insert(otherEdge);
            sharedEdges.insert(edge);
        }
    }
    edgeToEdgeMap[edge] = edgesSharingPoint;
}

Path findShortestPath(const map<Edge, set<Edge>>& edgeToEdgeMap, Edge startEdge) {
    queue<vector<Edge>> queue;
    Path path;

    for(Edge edge : edgeToEdgeMap.at(startEdge)){
        queue.push({startEdge, edge});
    }

    while(!queue.empty()){
        vector<Edge> currentPath = queue.front();
        Edge last = currentPath.back();
        for(Edge edge : edgeToEdgeMap.at(last)){
            if(edge == startEdge && currentPath.size() > 2 && !edgesShareCommonPoint(startEdge, currentPath.at(1), currentPath.back())){
                path = Path(currentPath);
                return path;
            } else if(!edgeAlreadySearched(edge, currentPath)){
                vector<Edge> newPath = currentPath;
                newPath.push_back(edge);
                queue.push(newPath);
            }
        }
        queue.pop();
    }

    return path;
}
