#ifndef BUTTON_CONTROLS_H
#define BUTTON_CONTROLS_H

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <set>
#include "Point.h"
using namespace std;

// Core mouse interaction functions called by main
void newPointClick(GLFWwindow* window, int button, int action, std::vector<Point> &clickedPoints, set<set<Point>> &allConnections, unordered_map<set<set<Point>>, optional<Point>, NestedSetPointHash>& intersectionPoints);
bool ConnectingPoints(GLFWwindow* window, int button, int action, std::vector<Point> &clickedPoints, std::set<Point> &currentConnection, std::set<std::set<Point>> &allConnections, bool &isConnecting);

// Helper helper utilities
bool hasActiveConnection(const std::set<Point>& activeConnections);
std::optional<Point> getNearestPoint(GLFWwindow* window, const std::vector<Point>& clickedPoints);
bool validClick(const Point& point, const Point& click);
void getCursorPositionInNDC(GLFWwindow* window, double& ndcX, double& ndcY);
void convertScreenToNDC(GLFWwindow* window, double screenX, double screenY, double& ndcX, double& ndcY);

// Console logging messages
void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY);
void getNearestPointMessage(const Point& nearestPoint);
void noNearbyPointMessage();

#endif // BUTTON_CONTROLS_H