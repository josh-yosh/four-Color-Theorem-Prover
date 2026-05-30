#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <optional>
#include "Point.h"
using namespace std;

constexpr double CLICK_THRESHOLD = 0.01; // Adjust as needed for click proximity

//add a new point to the screen
void newPointClick(GLFWwindow* window, int button, int action, vector<Point> &clickedPoints) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Fetch the virtual screen size (e.g., 800x600) which matches xpos/ypos
        int click_width, click_height;
        glfwGetWindowSize(window, &click_width, &click_height);

        float ndcX = (2.0f * (float)xpos) / (float)click_width - 1.0f;
        float ndcY = 1.0f - (2.0f * (float)ypos) / (float)click_height;

        clickedPoints.push_back(Point(ndcX, ndcY));

        pointClickedMessage(xpos, ypos, ndcX, ndcY);
    }
}

//checks if it's left clicked or released, and if it's a click near an existing point, then it will connect the two points
void ConnectingPoints(GLFWwindow* window, int button, int action, vector<Point> &clickedPoints, tuple<Point, Point> &currentConnection) {
    bool isLeftClick = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS);
    bool isLeftRelease = (button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_RELEASE);

    bool firstConnectionPointSet = !get<0>(currentConnection).isDefault(); // Check if the first point of the connection is already set
    bool secondConnectionPointSet = !get<1>(currentConnection).isDefault(); // Check if the second point of the connection is already set
    bool hasActiveConnection = firstConnectionPointSet && !secondConnectionPointSet; // Check if we already have a connection in progress
    int emptyPointIndex = indexOfEmptyPoint(currentConnection); // Get the index of the empty point in the connection tuple

    if (isLeftClick) {
        double ndcX, ndcY;
        getCursorPositionInNDC(window, ndcX, ndcY);

        optional<Point> nearestPointOpt = getNearestPoint(window, clickedPoints);
        if (nearestPointOpt) {
            Point nearestPoint = nearestPointOpt.value();
            cout << "Nearest Point Found: (" << nearestPoint.x << ", " << nearestPoint.y << ")\n";

        } else {
            cout << "No nearby point found for connection.\n";
        }
    } else if (isLeftRelease && hasActiveConnection) {
        
    }
}

int indexOfEmptyPoint(const tuple<Point, Point>& connection) {
    if (get<0>(connection).isDefault()) return 0;
    if (get<1>(connection).isDefault()) return 1;
    return -1; // No empty point
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

void getCursorPositionInNDC(GLFWwindow* window, double& xpos, double& ypos) {
    glfwGetCursorPos(window, &xpos, &ypos);
    double ndcX, ndcY;
    double screen_xpos, screen_ypos;

    pointClickedMessage(xpos, ypos, ndcX, ndcY);

    int click_width, click_height;
    glfwGetWindowSize(window, &click_width, &click_height);

    xpos = (2.0f * (float)xpos) / (float)click_width - 1.0f;
    ypos = 1.0f - (2.0f * (float)ypos) / (float)click_height;    
}

void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY) {
    cout << "Point 1 Clicked: (" << xpos << ", " << ypos 
                << ") -> NDC: (" << ndcX << ", " << ndcY << ")\n";

}