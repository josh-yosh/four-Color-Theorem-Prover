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

    // Iteratively prune dead ends: an edge is a dead end if, after ignoring
    // already-pruned edges, it no longer has valid neighbors on both endpoints.
    set<Edge> deadEnds;
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& [edge, neighbors] : edgeToEdgeMap) {
            if (deadEnds.count(edge)) continue;
            set<Point> pointsConnected;
            for (const Edge& n : neighbors) {
                if (deadEnds.count(n)) continue;
                for (const Point& p : n.endpoints) {
                    if (edge.endpoints.count(p)) pointsConnected.insert(p);
                }
            }
            if (pointsConnected.size() < 2) {
                deadEnds.insert(edge);
                changed = true;
            }
        }
    }

    set<AtomicEnclosure> allAtomicEnclosures;
    for(const Edge& startEdge : edges){
        if (deadEnds.count(startEdge)) continue;
        Path shortestPath = findShortestPath(edgeToEdgeMap, startEdge, deadEnds);
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

Path findShortestPath(const map<Edge, set<Edge>>& edgeToEdgeMap, Edge startEdge, const set<Edge>& deadEnds) {
    queue<vector<Edge>> queue;
    Path path;
    int count = 0;

    //start with every edge that connects to the beginning edge
    for(Edge edge : edgeToEdgeMap.at(startEdge)){
        if (!deadEnds.count(edge))
            queue.push({startEdge, edge});
    }

    //go until the queue is empty or shortest path found.
    while(!queue.empty()){
        vector<Edge> currentPath = queue.front();
        queue.pop();
        if(currentPath.size() > edgeToEdgeMap.size()) continue;

        Edge last = currentPath.back();

        //find all the possible ways from the last edge
        for(Edge edge : edgeToEdgeMap.at(last)){
            if(deadEnds.count(edge)) continue;
            if(edge == startEdge && currentPath.size() > 2 && !edgesShareCommonPoint(startEdge, currentPath.at(1), currentPath.back())){
                return Path(currentPath);
            } else if(!edgeAlreadySearched(edge, currentPath)){
                vector<Edge> newPath = currentPath;
                newPath.push_back(edge);
                queue.push(newPath);
            }
        }
        count++;
        cout << "iterations: " << count << "\n";

        if(count == 100000){
            // ── DEBUG: findShortestPath exhausted queue with no enclosure found ──
            auto printEdge = [](const Edge& e) {
                cout << "(" << e.p1().x << "," << e.p1().y << ")-(" << e.p2().x << "," << e.p2().y << ")";
            };

            cout << "\n=== findShortestPath FAILED (no enclosure found) ===\n";

            cout << "  startEdge: ";
            printEdge(startEdge);
            cout << "\n";

            cout << "  deadEnds (" << deadEnds.size() << "):\n";
            for (const Edge& e : deadEnds) {
                cout << "    "; printEdge(e); cout << "\n";
            }

            cout << "  edgeToEdgeMap (" << edgeToEdgeMap.size() << " edges):\n";
            for (const auto& [edge, neighbors] : edgeToEdgeMap) {
                cout << "    "; printEdge(edge); cout << " -> [";
                for (const Edge& n : neighbors) { printEdge(n); cout << " "; }
                cout << "]\n";
            }

            exit(1);
        }
    }

    return path;
}

bool cantMakeEnclosure(const map<Edge, set<Edge>>& edgeToEdgeMap, Edge startEdge){
    set<Point> pointsConnected;

    for(Edge edge : edgeToEdgeMap.at(startEdge)){ //get all edges connecting to the starting edge
        for(Point endpoint : edge.endpoints){ // grab the endpoints of those edges
            bool sharesPointWithStartEdge = startEdge.endpoints.count(endpoint) > 0; //see which endpoint it shares with the start edge point
            if(sharesPointWithStartEdge){ //if it does share a point:
                pointsConnected.insert(endpoint); // add that the the points of Pointsconnected
                if(pointsConnected.size() == 2) return false; //if both points have an edge, then it can make an enclosure
            }
        }
    }

    return true;
}

set<Edge> findDeadBranch(const map<Edge, set<Edge>>& edgeToEdgeMap, Edge deadEnd){
    set<Edge> deadBranch = {deadEnd};

    // Traverse from the dead-end edge until we reach an edge that branches
    Edge current = deadEnd;
    bool hasPrev = false;
    Edge prev; // will be set once we move forward

    while (true) {
        auto connectingEdges = edgeToEdgeMap.find(current);
        if (connectingEdges == edgeToEdgeMap.end()) break;

        const set<Edge>& neighbors = connectingEdges->second;

        const Edge& first = *neighbors.begin();
        const Edge& second = *std::next(neighbors.begin(), 1);
        //if there are 2 edges splitting off from the same edge
        cout << "neighbor size: " << neighbors.size() << "\n";
        cout << "share common point: " << edgesShareCommonPoint(deadEnd, first, second) << "\n";
        if (neighbors.size() > 2 || edgesShareCommonPoint(deadEnd, first, second)) break;

        // Choose the next edge: the neighbor that's not the previous edge (if any).
        Edge next;
        bool foundNext = false;
        for (const Edge& e : neighbors) {
            if (hasPrev && e == prev) continue;
            next = e;
            foundNext = true;
            break;
        }
        if (!foundNext) break;

        // Avoid cycles
        if (deadBranch.count(next)) break;

        deadBranch.insert(next);

        // advance
        prev = current;
        hasPrev = true;
        current = next;
    }

    return deadBranch;
}
