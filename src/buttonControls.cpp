#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include "lineAndPointLogic.h"
#include "Point.h"
#include "Edge.h"
#include "AtomicEnclosure.h"
#include "colorLogic.h"
using namespace std;

void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY);
void getNearestPointMessage(const Point& nearestPoint);
void noNearbyPointMessage();

// Add a new point to the screen
void newPointClick(GLFWwindow* window, int button, int action, set<Point>& clickedPoints, set<Edge>& allEdges) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        double ndcX, ndcY;
        convertScreenToNDC(window, xpos, ypos, ndcX, ndcY);

        // If click is near an existing edge, snap to it
        set<Point> possiblePointOnLine;
        for (const auto& edge : allEdges) {
            optional<Point> potentialPoint = closestPointOnLine(Point(ndcX, ndcY), edge.p1(), edge.p2());
            if (potentialPoint) {
                possiblePointOnLine.insert(*potentialPoint);
            }
        }

        if (!possiblePointOnLine.empty()) {
            Point pointOnLine = getNearestPointToTarget(possiblePointOnLine, Point(ndcX, ndcY));
            clickedPoints.insert(pointOnLine);
        } else {
            clickedPoints.insert(Point(ndcX, ndcY));
            pointClickedMessage(xpos, ypos, ndcX, ndcY);
        }
    }
}

// On click: start a connection. On release: complete it, splitting any crossed edges.
bool ConnectingPoints(GLFWwindow* window, int button, int action, set<Point>& clickedPoints, 
                        set<Point>& currentConnection, set<Edge>& allEdges, set<Point>& intersectionPoints,
                        bool& isConnecting, 
                        unordered_map<Point, set<Edge>>& pointToEdgeMap, set<AtomicEnclosure>& allAtomicEnclosures) {

    bool isLeftClick   = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS);
    bool isLeftRelease = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_RELEASE);
    bool activeConnectionExists = hasActiveConnection(currentConnection);

    if (isLeftClick && !activeConnectionExists) {
        isConnecting = true;
        optional<Point> nearestPointOpt = getNearestPoint(window, clickedPoints);
        if (nearestPointOpt) {
            currentConnection.insert(*nearestPointOpt);
            if (currentConnection.size() == 2) {
                allEdges.insert(Edge(*currentConnection.begin(), *next(currentConnection.begin())));
                currentConnection.clear();
                isConnecting = false;
            }
        } else {
            isConnecting = false;
        }
        return false;

    } else if (isLeftRelease && activeConnectionExists) {
        optional<Point> nearestPointOpt = getNearestPoint(window, clickedPoints);
        bool isSamePoint = (nearestPointOpt && currentConnection.count(*nearestPointOpt) > 0);

        if (nearestPointOpt && !isSamePoint) {
            currentConnection.insert(*nearestPointOpt);
            Edge currentEdge(*currentConnection.begin(), *next(currentConnection.begin()));

            set<Edge> edgesToRemove;
            set<Edge> edgesToAdd;
            bool originalConnectionNeeded = true;

            // Check intersections with existing edges
            for (const auto& edge : allEdges) {
                optional<Point> intersectionPoint = getIntersectionPoint(edge, currentEdge);
                optional<Point> nearbyPoint = nullopt;
                if (intersectionPoint) {
                    nearbyPoint = pointIsNearOtherPoints(*intersectionPoint, clickedPoints);
                }
                if (nearbyPoint) {
                    intersectionPoint = nearbyPoint;
                }
                if (intersectionPoint && !nearbyPoint) {
                    splitLineIntoSegments(edge, *intersectionPoint, edgesToAdd);
                    clickedPoints.insert(*intersectionPoint);
                    intersectionPoints.insert(*intersectionPoint);
                    edgesToRemove.insert(edge);
                    originalConnectionNeeded = false;
                }
            }

            // Check if any existing clicked points lie on the new edge
            for (const auto& point : clickedPoints) {
                bool pointIsNotEndpoint = !nearlyEqual(point, currentEdge.p1()) &&
                                          !nearlyEqual(point, currentEdge.p2());
                if (isPointOnLineSegment(point, currentEdge) && pointIsNotEndpoint) {
                    intersectionPoints.insert(point);
                    originalConnectionNeeded = false;
                }
            }

            if (!intersectionPoints.empty()) {
                breakLineIntoSegments(currentEdge, intersectionPoints, edgesToAdd);
                intersectionPoints.clear();
                originalConnectionNeeded = false;
            }

            for (const auto& edge : edgesToRemove) allEdges.erase(edge);
            for (const auto& edge : edgesToAdd)    allEdges.insert(edge);

            if (originalConnectionNeeded) {
                allEdges.insert(currentEdge);
            }

            addEdgeAndCheckForNewEnclosures(currentEdge, pointToEdgeMap, allAtomicEnclosures);
            currentConnection.clear();
            isConnecting = false;
            return true;
        } else {
            noNearbyPointMessage();
            currentConnection.clear();
            isConnecting = false;
            return false;
        }
    } else {
        isConnecting = false;
        return false;
    }
}

void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY) {
    cout << "Point Clicked: (" << xpos << ", " << ypos
         << ") -> NDC: (" << ndcX << ", " << ndcY << ")\n";
}

void getNearestPointMessage(const Point& nearestPoint) {
    cout << "Nearest Point Found: (" << nearestPoint.x << ", " << nearestPoint.y << ")\n";
}

void noNearbyPointMessage() {
    cout << "No nearby point found for connection.\n";
}
