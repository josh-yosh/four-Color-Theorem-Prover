#ifndef LINE_AND_POINT_LOGIC_H
#define LINE_AND_POINT_LOGIC_H
#define GLFW_INCLUDE_NONE

#include <unordered_map>
#include <optional>
#include <set>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Point.h"
#include "Edge.h"
using namespace std;

optional<Point> getIntersectionPoint(const Edge& e1, const Edge& e2);

optional<Point> closestPointOnLine(const Point& p, const Point& p1, const Point& p2);

void splitLineIntoSegments(const Edge& edge, Point intersectionPoint, set<Edge>& allEdges);

void breakLineIntoSegments(const Edge& edge, set<Point> intersectionPoints, set<Edge>& allEdges);

bool isPointOnLineSegment(const Point& p, const Edge& edge);

bool pointWithinSegments(const Point& p, const Point& p1, const Point& p2);

void getConstantsFromLine(const Edge& e1, const Edge& e2, float& a1, float& b1, float& c1, float& a2, float& b2, float& c2, float& determinant);

void getPointsFromEdges(const Edge& e1, const Edge& e2, Point& p1, Point& p2, Point& p3, Point& p4);

optional<Point> getNearestPoint(GLFWwindow* window, const set<Point>& clickedPoints);

void convertScreenToNDC(GLFWwindow* window, double screenX, double screenY, double& ndcX, double& ndcY);

void getCursorPositionInNDC(GLFWwindow* window, double& ndcX, double& ndcY);

bool validClick(const Point& point, const Point& click);

bool hasActiveConnection(const set<Point>& activeConnections);

optional<Point> pointIsNearOtherPoints(const Point& point, const set<Point>& otherPoints);

bool pointIsNearOtherPoint(const Point& point, const Point& point2);

Point getNearestPointToTarget(const set<Point>& points, const Point& target);

#endif // LINE_AND_POINT_LOGIC_H
