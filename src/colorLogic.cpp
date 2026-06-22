#include <set>
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "colorLogic.h"
#include "AtomicEnclosure.h"
#include "Path.h"


// ── Used functions ────────────────────────────────────────────────────────────

// Iteratively removes dead-end (degree-1) edges. A vertex touched by only one
// edge can't be part of any face boundary, but the face tracer would still walk
// out to it and back, producing a self-touching (non-simple) polygon that the
// ear-clipping triangulator can't fill. Removing one dead-end can expose a new
// one, so we keep trimming "leaves" until the set stabilises.
static set<Edge> pruneDanglingEdges(const set<Edge>& edges) {
    set<Edge> remaining = edges;

    map<Point, int> degree;
    for (const Edge& e : remaining) {
        degree[e.p1()]++;
        degree[e.p2()]++;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = remaining.begin(); it != remaining.end(); ) {
            const Point& a = it->p1();
            const Point& b = it->p2();
            if (degree[a] == 1 || degree[b] == 1) {
                degree[a]--;
                degree[b]--;
                it = remaining.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }
    }

    return remaining;
}

set<AtomicEnclosure> findAtomicEnclosures(const set<Edge>& allEdges) {
    // Drop dead-end edges first so every traced face is a simple polygon.
    set<Edge> edges = pruneDanglingEdges(allEdges);
    if (edges.size() != allEdges.size()) {
        cout << "[findAtomicEnclosures] Pruned " << (allEdges.size() - edges.size())
             << " dead-end edge(s) before tracing\n";
    }

    // Build adjacency: point -> list of neighbors sorted by angle (CCW)
    map<Point, vector<Point>> adjacency;
    for (const Edge& e : edges) {
        adjacency[e.p1()].push_back(e.p2());
        adjacency[e.p2()].push_back(e.p1());
    }
    for (auto& [p, neighbors] : adjacency) {
        sort(neighbors.begin(), neighbors.end(), [&p](const Point& a, const Point& b) {
            double angleA = atan2(a.y - p.y, a.x - p.x);
            double angleB = atan2(b.y - p.y, b.x - p.x);
            return angleA < angleB;
        });
    }

    // Trace faces using half-edge rotation:
    // For directed half-edge (u -> v), the next half-edge in the same interior face
    // is (v -> w) where w is the neighbor of v that comes just AFTER u in CW order
    // (= just BEFORE u in CCW order).
    set<pair<Point,Point>> usedHalfEdges;
    set<AtomicEnclosure> allAtomicEnclosures;

    auto findNeighborIndex = [&](const Point& center, const Point& target) -> int {
        const vector<Point>& neighbors = adjacency.at(center);
        for (int i = 0; i < (int)neighbors.size(); i++) {
            // Use exact float comparison: points come from the same set<Point> storage
            if (neighbors[i].x == target.x && neighbors[i].y == target.y) return i;
        }
        return -1;
    };

    cout << "\n[findAtomicEnclosures] Starting — " << edges.size() << " edges\n";

    for (const Edge& startEdge : edges) {
        for (int dir = 0; dir < 2; dir++) {
            Point from = (dir == 0) ? startEdge.p1() : startEdge.p2();
            Point to   = (dir == 0) ? startEdge.p2() : startEdge.p1();

            if (usedHalfEdges.count({from, to})) continue;

            cout << "  [Half-edge] (" << from.x << ", " << from.y << ") -> ("
                 << to.x << ", " << to.y << ")\n";

            // Trace the face this half-edge belongs to
            vector<Point> facePoints;
            set<Edge> faceEdges;
            Point cur_from = from, cur_to = to;
            bool valid = true;

            for (int steps = 0; steps < (int)edges.size() * 2 + 1; steps++) {
                if (usedHalfEdges.count({cur_from, cur_to})) break;
                usedHalfEdges.insert({cur_from, cur_to});
                facePoints.push_back(cur_from);
                faceEdges.insert(Edge(cur_from, cur_to));

                // At cur_to, find cur_from in the sorted neighbor list
                int idx = findNeighborIndex(cur_to, cur_from);
                if (idx < 0) {
                    cout << "    [!] Neighbor not found for ("
                         << cur_to.x << ", " << cur_to.y << ") — aborting face\n";
                    valid = false;
                    break;
                }

                // Next in CW order = previous in CCW order
                const vector<Point>& neighbors = adjacency.at(cur_to);
                int nextIdx = (idx - 1 + (int)neighbors.size()) % (int)neighbors.size();
                Point next_to = neighbors[nextIdx];

                cout << "    Step: (" << cur_from.x << ", " << cur_from.y << ") -> ("
                     << cur_to.x << ", " << cur_to.y << ") next -> ("
                     << next_to.x << ", " << next_to.y << ")\n";

                cur_from = cur_to;
                cur_to = next_to;
            }

            if (!valid || facePoints.size() < 3) {
                cout << "  [Skip] Invalid or too few points (" << facePoints.size() << ")\n";
                continue;
            }

            // Compute signed area to reject the outer (unbounded) face.
            // Interior faces are wound CW (negative signed area in standard coords).
            double signedArea = 0.0;
            int n = facePoints.size();
            for (int i = 0; i < n; i++) {
                const Point& a = facePoints[i];
                const Point& b = facePoints[(i + 1) % n];
                signedArea += (a.x * b.y - b.x * a.y);
            }
            signedArea /= 2.0;

            cout << "  [Face] " << facePoints.size() << " vertices, signed area = " << signedArea << " -> ";

            // Also reject degenerate faces: the face tracer can backtrack along
            // dead-end (degree-1) edges producing a self-intersecting path with
            // near-zero area. A real interior face needs meaningful area.
            if (signedArea <= 1e-6) {
                cout << "OUTER/DEGENERATE FACE (skipped)\n";
                continue;
            }

            cout << "INTERIOR FACE (kept)\n";
            cout << "  [Vertices]:";
            for (const Point& p : facePoints)
                cout << " (" << p.x << ", " << p.y << ")";
            cout << "\n";

            set<Point> facePointSet(facePoints.begin(), facePoints.end());
            allAtomicEnclosures.insert(AtomicEnclosure(facePointSet, faceEdges));
        }
    }

    cout << "[findAtomicEnclosures] Done — " << allAtomicEnclosures.size() << " enclosure(s) found\n\n";

    return allAtomicEnclosures;
}


// ── Unused functions ──────────────────────────────────────────────────────────

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

// Returns the shared point between two edges (the point they both have as an endpoint).
static Point sharedPoint(const Edge& a, const Edge& b) {
    for (const Point& p : a.endpoints)
        if (b.hasEndpoint(p)) return p;
    return *a.endpoints.begin(); // fallback (shouldn't happen)
}

Path findShortestPath(const map<Edge, set<Edge>>& edgeToEdgeMap, Edge startEdge, const set<Edge>& deadEnds) {
    // Each queue entry: (path of edges, set of interior points already visited)
    struct PathState {
        vector<Edge> edges;
        set<Point> visitedPoints;
    };
    queue<PathState> bfsQueue;
    Path path;
    int count = 0;

    //start with every edge that connects to the beginning edge
    for(const Edge& edge : edgeToEdgeMap.at(startEdge)){
        if (!deadEnds.count(edge)) {
            Point entry = sharedPoint(startEdge, edge);
            bfsQueue.push({{startEdge, edge}, {entry}});
        }
    }

    //go until the queue is empty or shortest path found.
    while(!bfsQueue.empty()){
        PathState state = bfsQueue.front();
        bfsQueue.pop();
        vector<Edge>& currentPath = state.edges;
        set<Point>& visited = state.visitedPoints;

        if(currentPath.size() > edgeToEdgeMap.size()) continue;

        Edge last = currentPath.back();

        //find all the possible ways from the last edge
        for(const Edge& edge : edgeToEdgeMap.at(last)){
            if(deadEnds.count(edge)) continue;
            if(edge == startEdge && currentPath.size() > 2 && !edgesShareCommonPoint(startEdge, currentPath.at(1), currentPath.back())){
                return Path(currentPath);
            } else if(!edgeAlreadySearched(edge, currentPath)){
                // Find the point we'd be moving through (the shared point between last and edge)
                Point nextPoint = sharedPoint(last, edge);
                // Skip if we'd revisit an interior point (prevents combinatorial explosion)
                if (visited.count(nextPoint)) continue;
                PathState newState = state;
                newState.edges.push_back(edge);
                newState.visitedPoints.insert(nextPoint);
                bfsQueue.push(std::move(newState));
            }
        }
        count++;
        cout << "iterations: " << count << "\n";

        if(count == 10000){
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

            cout << "  queue (" << bfsQueue.size() << " paths):\n";
            {
                auto qCopy = bfsQueue;
                int pathIdx = 0;
                while (!qCopy.empty()) {
                    const vector<Edge>& p = qCopy.front().edges;
                    cout << "    [" << pathIdx++ << "] (" << p.size() << " edges): ";
                    for (const Edge& e : p) { printEdge(e); cout << " -> "; }
                    cout << "\n";
                    qCopy.pop();
                }
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
