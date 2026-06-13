#include <set>
#include <vector>
#include <unordered_map>
#include <queue>
#include "colorLogic.h"
#include "AtomicEnclosure.h"


set<AtomicEnclosure> findAtomicEnclosures(const set<Point>& points, const set<Edge>& edges) {
    unordered_map<Point, set<Edge>> pointToEdgeMap = createPointToEdgeMap(points, edges);
    set<AtomicEnclosure> allAtomicEnclosures;
    
    for(Point point : points){
        set<vector<Edge>> shortestPaths = findShortestPaths(pointToEdgeMap, point);

        for(vector<Edge> path : shortestPaths){
            set<Point> pointsInPath;
            set<Edge> edgesInEnclosure;

            for(Edge edge : path){
                pointsInPath.insert(edge.endpoints.begin(), edge.endpoints.end());
            }

            edgesInEnclosure.insert(path.begin(), path.end());
            allAtomicEnclosures.insert(AtomicEnclosure(pointsInPath, edgesInEnclosure));
        }
    }

    return allAtomicEnclosures;
}

void addEdgeAndCheckForNewEnclosures(Edge newEdge, unordered_map<Point, set<Edge>>& pointToEdgeMap, set<AtomicEnclosure>& allAtomicEnclosures){
    addEdgeToEdgeMap(newEdge, pointToEdgeMap);

    set<vector<Edge>> newEnclosures;
    for(Point endpoint : newEdge.endpoints){
        set<vector<Edge>> paths = findShortestPaths(pointToEdgeMap, endpoint);
        newEnclosures.insert(paths.begin(), paths.end());
    }

    set<AtomicEnclosure> newAtomicEnclosures = convertSetOfPathsToAtomicEnclosure(newEnclosures);
    allAtomicEnclosures.insert(newAtomicEnclosures.begin(), newAtomicEnclosures.end());
}

set<AtomicEnclosure> convertSetOfPathsToAtomicEnclosure(set<vector<Edge>> newEnclosures){
    set<AtomicEnclosure> allAtomicEnclosures;

    for(vector<Edge> path : newEnclosures){
        set<Point> pointsInPath;
        set<Edge> edgesInEnclosure;

        for(Edge edge : path){
            pointsInPath.insert(edge.endpoints.begin(), edge.endpoints.end());
        }

        edgesInEnclosure.insert(path.begin(), path.end());
        allAtomicEnclosures.insert(AtomicEnclosure(pointsInPath, edgesInEnclosure));
    }

    return allAtomicEnclosures;
} 

set<vector<Edge>> findShortestPaths(const unordered_map<Point, set<Edge>>& pointToEdgeMap, Point startPoint) {
    queue<vector<Edge>> queue;
        set<vector<Edge>> paths;
        set<Point> explored;
        int shortestPathLength = 99;
        int currentPathLength = 1;

        for(Edge edge : pointToEdgeMap.at(startPoint)){
            queue.push({edge});
        }

        while(!queue.empty() && currentPathLength <= shortestPathLength){
            currentPathLength += 1;
            vector<Edge> current = queue.front();
            Edge last = current.back();
            for(Point endpoint : last.endpoints){
                if(endpoint == startPoint){
                    paths.insert(current);
                    shortestPathLength = currentPathLength;
                } else if(explored.count(endpoint) < 1){
                    set<Edge> allEdgesFromLastPoint = pointToEdgeMap.at(endpoint);
                    for(Edge edge : allEdgesFromLastPoint){
                        vector<Edge> newPath = current;
                        newPath.push_back(edge);
                        queue.push(newPath);
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