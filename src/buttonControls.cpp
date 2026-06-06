#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include "lineAndPointLogic.h"
#include "Point.h"
using namespace std;

constexpr double CLICK_THRESHOLD = 0.025; // Adjust as needed for click proximity
void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY);
void getNearestPointMessage(const Point& nearestPoint);
void noNearbyPointMessage();

//add a new point to the screen
void newPointClick(GLFWwindow* window, int button, int action, set<Point> &clickedPoints, set<set<Point>> &allEdges) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Fetch the virtual screen size (e.g., 800x600) which matches xpos/ypos
        int click_width, click_height;
        glfwGetWindowSize(window, &click_width, &click_height);

        // Convert screen coordinates to NDC
        double ndcX, ndcY;
        convertScreenToNDC(window, xpos, ypos, ndcX, ndcY);

        //checks if the click is near an existing line, if so, it will add a point on the line and split the line into two segments
        optional<Point> pointOnLine = nullopt;
        for (const auto& connection : allEdges) {
            vector<Point> connPoints(connection.begin(), connection.end());
            optional<Point> potentialPoint = closestPointOnLine(Point(ndcX, ndcY), connPoints[0], connPoints[1]);
            if (potentialPoint != nullopt) {
                pointOnLine = potentialPoint;
                break;
            }
        }

        if (pointOnLine) {
            clickedPoints.insert(*pointOnLine);
        } else {
            clickedPoints.insert(Point(ndcX, ndcY));
            pointClickedMessage(xpos, ypos, ndcX, ndcY);
        }
    }
}

//checks if it's left clicked or released, and if it's a click near an existing point, then it will connect the two points
bool ConnectingPoints(GLFWwindow* window, int button, int action, set<Point> &clickedPoints, set<Point> &currentConnection, set<set<Point>> &allEdges, set<Point> &intersectionPoints, bool &isConnecting) {
    bool isLeftClick = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS);
    bool isLeftRelease = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_RELEASE);

    bool activeConnectionExists = hasActiveConnection(currentConnection); // Check if we already have a connection in progress
    bool returnValue = false;

    if (isLeftClick && !activeConnectionExists) {
        double ndcX, ndcY;
        getCursorPositionInNDC(window, ndcX, ndcY);
        isConnecting = true; // We are in the process of connecting points

        optional<Point> nearestPointOpt = getNearestPoint(window, clickedPoints);
        if (nearestPointOpt) {
            Point nearestPoint = nearestPointOpt.value();
            // getNearestPointMessage(nearestPoint);
            currentConnection.insert(nearestPoint); // Add the nearest point to the connection set
            if (currentConnection.size() == 2) {
                allEdges.insert(currentConnection); // Store the completed connection
                currentConnection.clear(); // Reset for the next connection
                isConnecting = false; // Connection completed
            }
            returnValue = false; // Connection handled

        } else {
            // noNearbyPointMessage();
            isConnecting = false;
            returnValue = false; // No connection made
        }
    } else if (isLeftRelease && activeConnectionExists) {
        double ndcX, ndcY;
        getCursorPositionInNDC(window, ndcX, ndcY);

        optional<Point> nearestPointOpt = getNearestPoint(window, clickedPoints);
        bool isSamePoint = (nearestPointOpt && currentConnection.count(nearestPointOpt.value()) > 0);
        if (nearestPointOpt && !isSamePoint) {
            Point nearestPoint = nearestPointOpt.value();
            // getNearestPointMessage(nearestPoint);
            currentConnection.insert(nearestPoint); // Add the nearest point to the connection set
            set<set<Point>> edgesToRemove; // To store new edges after splitting
            set<set<Point>> edgesToAdd; // To store new edges after splitting
            bool originalConnectionNeeded = true; // Flag to determine if the original connection should be added after processing intersections

            //check if the current connection intersects with any existing edges, if so, split the existing edge into two segments and add the intersection point to the clicked points for rendering
            for (const auto& connection : allEdges) {
                optional<Point> intersectionPoint = getIntersectionPoint(connection, currentConnection);
                optional<Point> nearbyPoint = nullopt;
                if(intersectionPoint.has_value()) {
                    nearbyPoint = pointIsNearOtherPoints(intersectionPoint.value(), clickedPoints);
                    cout << "nearby point check for intersection: " << nearbyPoint.has_value() << "\n";
                }
                if(nearbyPoint) {
                    intersectionPoint = nearbyPoint;
                }

                if (intersectionPoint.has_value() && !nearbyPoint.has_value()) {
                    splitLineIntoSegments(connection, *intersectionPoint, edgesToAdd);
                    clickedPoints.insert(*intersectionPoint); // Add the intersection point to the clicked points for rendering
                    intersectionPoints.insert(*intersectionPoint);
                    edgesToRemove.insert(connection); // Mark the original edge for removal
                    originalConnectionNeeded = false; // Original connection will be replaced by the two new segments
                }
            }

            for(const auto& point : clickedPoints) {
                bool pointIsNotEndpoint = !nearlyEqual(point, *currentConnection.begin()) && !nearlyEqual(point, *currentConnection.rbegin());
                if(isPointOnLineSegment(point, currentConnection) && pointIsNotEndpoint) {   
                    intersectionPoints.insert(point);
                    originalConnectionNeeded = false; // If there's a point on the line, we will be adding segments, so the original connection won't be needed
                }
            }

            set<Point> dupIntersectionPoints(intersectionPoints.begin(), intersectionPoints.end()); // Create a copy to iterate over
            cout << "Intersection Points: " << intersectionPoints.size() << "\n";
            if(intersectionPoints.size() > 0){
                breakLineIntoSegments(currentConnection, dupIntersectionPoints, edgesToAdd);
                intersectionPoints.clear(); // Clear intersection points after processing
                originalConnectionNeeded = false; // If there are intersection points, we will be adding segments, so the original connection won't be needed
            }

            cout << "Edges to Remove: " << edgesToRemove.size() << ", Edges to Add: " << edgesToAdd.size() << "\n";

            // Remove original edges after processing all intersections to avoid modifying the set while iterating
            for (const auto& edge : edgesToRemove) {
                allEdges.erase(edge);
            }
            for (const auto& edge : edgesToAdd) {
                allEdges.insert(edge);
            }

            if (originalConnectionNeeded) {
                cout << "original connection needed, adding\n";
                allEdges.insert(currentConnection); // Store the completed connection
            }
            currentConnection.clear(); // Clear the connection state after release regardless of success
            returnValue = true; // Connection handled
        } else {
            noNearbyPointMessage();
            currentConnection.clear(); // Clear the connection state on release regardless of success
            returnValue = false; // No connection made
        }

        isConnecting = false; // Connection completed
    } else {
        isConnecting = false; // Connection completed
        returnValue = false; // Not a connection event
    }
    cout << "-------------------------------\n";
    return returnValue;
}

void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY) {
    cout << "Point 1 Clicked: (" << xpos << ", " << ypos 
                << ") -> NDC: (" << ndcX << ", " << ndcY << ")\n";

}

void getNearestPointMessage(const Point& nearestPoint) {
    cout << "Nearest Point Found: (" << nearestPoint.x << ", " << nearestPoint.y << ")\n";
}

void noNearbyPointMessage() {
    cout << "No nearby point found for connection.\n";
}




