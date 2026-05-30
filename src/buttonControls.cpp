#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <optional>
#include <set>
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
void newPointClick(GLFWwindow* window, int button, int action, vector<Point> &clickedPoints) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Fetch the virtual screen size (e.g., 800x600) which matches xpos/ypos
        int click_width, click_height;
        glfwGetWindowSize(window, &click_width, &click_height);

        // Convert screen coordinates to NDC
        double ndcX, ndcY;
        convertScreenToNDC(window, xpos, ypos, ndcX, ndcY);

        clickedPoints.push_back(Point(ndcX, ndcY));
        pointClickedMessage(xpos, ypos, ndcX, ndcY);
    }
}

//checks if it's left clicked or released, and if it's a click near an existing point, then it will connect the two points
bool ConnectingPoints(GLFWwindow* window, int button, int action, vector<Point> &clickedPoints, set<Point> &currentConnection, set<set<Point>> &allConnections) {
    bool isLeftClick = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS);
    bool isLeftRelease = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_RELEASE);

    bool activeConnectionExists = hasActiveConnection(currentConnection); // Check if we already have a connection in progress

    if (isLeftClick && !activeConnectionExists) {
        double ndcX, ndcY;
        getCursorPositionInNDC(window, ndcX, ndcY);

        optional<Point> nearestPointOpt = getNearestPoint(window, clickedPoints);
        if (nearestPointOpt) {
            Point nearestPoint = nearestPointOpt.value();
            getNearestPointMessage(nearestPoint);
            currentConnection.insert(nearestPoint); // Add the nearest point to the connection set
            if (currentConnection.size() == 2) {
                allConnections.insert(currentConnection); // Store the completed connection
                currentConnection.clear(); // Reset for the next connection
            }
            return true; // Connection handled

        } else {
            noNearbyPointMessage();
            return false; // No connection made
        }
    } else if (isLeftRelease && activeConnectionExists) {
        double ndcX, ndcY;
        getCursorPositionInNDC(window, ndcX, ndcY);

        optional<Point> nearestPointOpt = getNearestPoint(window, clickedPoints);
        if (nearestPointOpt) {
            Point nearestPoint = nearestPointOpt.value();
            getNearestPointMessage(nearestPoint);
            currentConnection.insert(nearestPoint); // Add the nearest point to the connection set
            currentConnection.clear(); // Clear the connection state after release regardless of success

            return true; // Connection handled
        } else {
            noNearbyPointMessage();
            currentConnection.clear(); // Clear the connection state on release regardless of success

            return false; // No connection made
        }
    }
    return false; // Not a connection event
}

bool hasActiveConnection(const set<Point>& activeConnections) {
    return !activeConnections.empty();
}

// Returns the first point that is near the click position, or nullopt if none are close enough
optional<Point> getNearestPoint(GLFWwindow* window, const vector<Point>& clickedPoints) {
    double ndcX, ndcY;
    getCursorPositionInNDC(window, ndcX, ndcY);

    // Search if point is near clicked point.
    for (const auto& point : clickedPoints) {
        if (validClick(point, Point(ndcX, ndcY))) {
            return point; // Automatically wrapped in optional
        }
    }

    // Return "no point" safely
    return nullopt; 
}

// determines if a click is close enough to a point to be considered a valid selection
bool validClick(const Point& point, const Point& click) {
    double distanceSquared = (point.x - click.x) * (point.x - click.x) + (point.y - click.y) * (point.y - click.y);
    return distanceSquared < (CLICK_THRESHOLD * CLICK_THRESHOLD); // Compare squared distances to avoid sqrt
}

void getCursorPositionInNDC(GLFWwindow* window, double& ndcX, double& ndcY) {
    double screenX, screenY;
    glfwGetCursorPos(window, &screenX, &screenY);

    int width, height;
    glfwGetWindowSize(window, &width, &height); // Must be WindowSize to match CursorPos!

    // Calculate NDC (-1.0 to 1.0)
    ndcX = (2.0 * screenX) / (double)width - 1.0;
    ndcY = 1.0 - (2.0 * screenY) / (double)height;    
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

void convertScreenToNDC(GLFWwindow* window, double screenX, double screenY, double& ndcX, double& ndcY) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ndcX = (2.0f * (float)screenX) / (float)width - 1.0f;
    ndcY = 1.0f - (2.0f * (float)screenY) / (float)height;
}


