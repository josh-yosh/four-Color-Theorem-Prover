#ifndef BUTTON_CONTROLS_H
#define BUTTON_CONTROLS_H

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <set>
#include "Point.h"
#include "Edge.h"
using namespace std;

// Core mouse interaction functions called by main
void newPointClick(GLFWwindow* window, int button, int action, set<Point>& clickedPoints, set<Edge>& allEdges);
bool ConnectingPoints(GLFWwindow* window, int button, int action, set<Point>& clickedPoints, set<Point>& currentConnection, set<Edge>& allEdges, set<Point>& intersectionPoints, bool& isConnecting);

// Helper utilities
bool hasActiveConnection(const set<Point>& activeConnections);
optional<Point> getNearestPoint(GLFWwindow* window, const set<Point>& clickedPoints);
bool validClick(const Point& point, const Point& click);
void getCursorPositionInNDC(GLFWwindow* window, double& ndcX, double& ndcY);
void convertScreenToNDC(GLFWwindow* window, double screenX, double screenY, double& ndcX, double& ndcY);

// Console logging messages
void pointClickedMessage(double xpos, double ypos, double ndcX, double ndcY);
void getNearestPointMessage(const Point& nearestPoint);
void noNearbyPointMessage();

#endif // BUTTON_CONTROLS_H
