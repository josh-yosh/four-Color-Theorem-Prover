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

constexpr double CLICK_THRESHOLD = 0.05; // Adjust as needed for click proximity

void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY);
void getNearestPointMessage(const Point& nearestPoint);
void noNearbyPointMessage();
bool hasActiveConnection(const set<Point>& activeConnections);
void getCursorPositionInNDC(GLFWwindow* window, double& ndcX, double& ndcY);
bool validClick(const Point& point, const Point& click);
optional<Point> getNearestPoint(GLFWwindow* window, const vector<Point>& clickedPoints);
void convertScreenToNDC(GLFWwindow* window, double screenX, double screenY, double& ndcX, double& ndcY);

//add a new point to the screen
void newPointClick(GLFWwindow* window, int button, int action, vector<Point> &clickedPoints, set<set<Point>> &allEdges) {
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
            clickedPoints.push_back(*pointOnLine);
        } else {
            clickedPoints.push_back(Point(ndcX, ndcY));
            pointClickedMessage(xpos, ypos, ndcX, ndcY);
        }
    }
}

//checks if it's left clicked or released, and if it's a click near an existing point, then it will connect the two points
bool ConnectingPoints(GLFWwindow* window, int button, int action, vector<Point> &clickedPoints, set<Point> &currentConnection, set<set<Point>> &allEdges, set<Point> &intersectionPoints, bool &isConnecting) {
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
            getNearestPointMessage(nearestPoint);
            currentConnection.insert(nearestPoint); // Add the nearest point to the connection set
            if (currentConnection.size() == 2) {
                allEdges.insert(currentConnection); // Store the completed connection
                currentConnection.clear(); // Reset for the next connection
                isConnecting = false; // Connection completed
            }
            returnValue = false; // Connection handled

        } else {
            noNearbyPointMessage();
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
            getNearestPointMessage(nearestPoint);
            currentConnection.insert(nearestPoint); // Add the nearest point to the connection set
            set<set<Point>> edgesToRemove; // To store new edges after splitting
            set<set<Point>> edgesToAdd; // To store new edges after splitting

            for (const auto& connection : allEdges) {
                optional<Point> intersectionPoint = getIntersectionPoint(connection, currentConnection);
                if (intersectionPoint.has_value()) {
                    splitLineIntoSegments(connection, *intersectionPoint, edgesToAdd);
                    clickedPoints.push_back(*intersectionPoint); // Add the intersection point to the clicked points for rendering
                    intersectionPoints.insert(*intersectionPoint);
                    edgesToRemove.insert(connection); // Mark the original edge for removal
                } 
                
            }
            
            //print intersection points for debugging
            for (const auto& ip : intersectionPoints) {
                cout << "Intersection Point: (" << ip.x << ", " << ip.y << ")\n";
            }

            vector<Point> dupIntersectionPoints(intersectionPoints.begin(), intersectionPoints.end()); // Create a copy to iterate over
            if(intersectionPoints.size() > 0){
                breakLineIntoSegments(currentConnection, dupIntersectionPoints, edgesToAdd);
                edgesToRemove.insert(currentConnection); // Mark the original edge for removal
                intersectionPoints.clear(); // Clear intersection points after processing
            }

            // Remove original edges after processing all intersections to avoid modifying the set while iterating
            for (const auto& edge : edgesToRemove) {
                allEdges.erase(edge);
            }
            for (const auto& edge : edgesToAdd) {
                allEdges.insert(edge);
            }

            allEdges.insert(currentConnection); // Store the completed connection
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




