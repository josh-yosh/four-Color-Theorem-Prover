#include <unordered_map>
#include <optional>
#include <set>
#include <vector>
#include <limits>
#include "lineAndPointLogic.h"
#include "Point.h"
#include "Edge.h"
using namespace std;

constexpr double CLICK_THRESHOLD  = 0.015;
constexpr double CLICK_THRESHOLD2 = 0.025;

optional<Point> getIntersectionPoint(const Edge& e1, const Edge& e2) {
    Point p1, p2, p3, p4;
    getPointsFromEdges(e1, e2, p1, p2, p3, p4);
    float a1, b1, c1, a2, b2, c2, determinant;
    getConstantsFromLine(e1, e2, a1, b1, c1, a2, b2, c2, determinant);

    bool collinear = (determinant == 0);
    bool intersectionExists = false;
    float x, y;
    if (!collinear) {
        x = (b2 * c1 - b1 * c2) / determinant;
        y = (a1 * c2 - a2 * c1) / determinant;
        Point intersectionPoint{x, y};
        bool intersectionPointNotEndpoint =
            !nearlyEqual(intersectionPoint, p1) && !nearlyEqual(intersectionPoint, p2) &&
            !nearlyEqual(intersectionPoint, p3) && !nearlyEqual(intersectionPoint, p4);
        intersectionExists = pointWithinSegments(intersectionPoint, p1, p2) &&
                             pointWithinSegments(intersectionPoint, p3, p4) &&
                             intersectionPointNotEndpoint;
    }

    if (intersectionExists) {
        return Point{x, y};
    }
    return std::nullopt;
}

void splitLineIntoSegments(const Edge& edge, Point intersectionPoint, set<Edge>& allEdges) {
    Point p1 = edge.p1();
    Point p2 = edge.p2();

    if (nearlyEqual(p1, intersectionPoint) || nearlyEqual(p2, intersectionPoint)) {
        return; // Intersection is at an endpoint — no split needed
    }

    allEdges.insert(Edge(p1, intersectionPoint));
    allEdges.insert(Edge(intersectionPoint, p2));
}

void breakLineIntoSegments(const Edge& edge, set<Point> intersectionPoints, set<Edge>& allEdges) {
    if (!edge.isComplete()) return;

    vector<Point> pointsOnLine(intersectionPoints.begin(), intersectionPoints.end());
    pointsOnLine.push_back(edge.p1());
    pointsOnLine.push_back(edge.p2());

    Point p1 = edge.p1();
    sort(pointsOnLine.begin(), pointsOnLine.end(), [&](const Point& a, const Point& b) {
        float da = (a.x - p1.x) * (a.x - p1.x) + (a.y - p1.y) * (a.y - p1.y);
        float db = (b.x - p1.x) * (b.x - p1.x) + (b.y - p1.y) * (b.y - p1.y);
        return da < db;
    });

    pointsOnLine.erase(std::unique(pointsOnLine.begin(), pointsOnLine.end()), pointsOnLine.end());

    for (int i = 0; i < (int)pointsOnLine.size() - 1; i++) {
        allEdges.insert(Edge(pointsOnLine[i], pointsOnLine[i + 1]));
    }
}

bool isPointOnLineSegment(const Point& p, const Edge& edge) {
    Point p1 = edge.p1();
    Point p2 = edge.p2();

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float epsilon = 0.025f;
    float segmentLengthSq = dx * dx + dy * dy;

    if (segmentLengthSq == 0.0f) return false;

    float t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / segmentLengthSq;
    t = std::clamp(t, 0.0f, 1.0f);

    Point closestPoint{p1.x + t * dx, p1.y + t * dy};

    float distSq = p.distanceSquared(closestPoint);
    bool lessThanEpsilon    = distSq <= epsilon * epsilon;
    bool withinSegmentBounds = pointWithinSegments(closestPoint, p1, p2);
    bool pointIsNotEndpoint  = !nearlyEqual(closestPoint, p1) && !nearlyEqual(closestPoint, p2);
    return lessThanEpsilon && withinSegmentBounds && pointIsNotEndpoint;
}

bool pointWithinSegments(const Point& p, const Point& p1, const Point& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    float t;
    if (fabs(dx) > fabs(dy)) {
        t = (p.x - p1.x) / dx;
    } else if (fabs(dy) > 0) {
        t = (p.y - p1.y) / dy;
    } else {
        return false; // degenerate segment
    }

    return t >= 0.0f && t <= 1.0f;
}

void getPointsFromEdges(const Edge& e1, const Edge& e2, Point& p1, Point& p2, Point& p3, Point& p4) {
    p1 = e1.p1();
    p2 = e1.p2();
    p3 = e2.p1();
    p4 = e2.p2();
}

void getConstantsFromLine(const Edge& e1, const Edge& e2, float& a1, float& b1, float& c1, float& a2, float& b2, float& c2, float& determinant) {
    Point p1 = e1.p1(), p2 = e1.p2();
    Point p3 = e2.p1(), p4 = e2.p2();

    a1 = p2.y - p1.y;
    b1 = p1.x - p2.x;
    c1 = a1 * p1.x + b1 * p1.y;

    a2 = p4.y - p3.y;
    b2 = p3.x - p4.x;
    c2 = a2 * p3.x + b2 * p3.y;

    determinant = a1 * b2 - a2 * b1;
}

optional<Point> closestPointOnLine(const Point& p, const Point& p1, const Point& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float epsilon = 0.05f;
    float segmentLengthSq = dx * dx + dy * dy;

    if (segmentLengthSq == 0.0f) {
        return std::sqrt(p.distanceSquared(p1)) <= epsilon ? optional<Point>(p1) : nullopt;
    }

    float t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / segmentLengthSq;
    t = std::clamp(t, 0.0f, 1.0f);
    Point closestPoint{p1.x + t * dx, p1.y + t * dy};

    return p.distanceSquared(closestPoint) <= epsilon * epsilon ? optional<Point>(closestPoint) : nullopt;
}

bool hasActiveConnection(const set<Point>& activeConnections) {
    return !activeConnections.empty();
}

bool validClick(const Point& point, const Point& click) {
    double distanceSquared = (point.x - click.x) * (point.x - click.x) +
                             (point.y - click.y) * (point.y - click.y);
    return distanceSquared < (CLICK_THRESHOLD * CLICK_THRESHOLD);
}

bool pointIsNearOtherPoint(const Point& point, const Point& point2) {
    double distanceSquared = (point.x - point2.x) * (point.x - point2.x) +
                             (point.y - point2.y) * (point.y - point2.y);
    return distanceSquared < (CLICK_THRESHOLD2 * CLICK_THRESHOLD2);
}

void convertScreenToNDC(GLFWwindow* window, double screenX, double screenY, double& ndcX, double& ndcY) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ndcX = (2.0f * (float)screenX) / (float)width - 1.0f;
    ndcY = 1.0f - (2.0f * (float)screenY) / (float)height;
}

void getCursorPositionInNDC(GLFWwindow* window, double& ndcX, double& ndcY) {
    double screenX, screenY;
    glfwGetCursorPos(window, &screenX, &screenY);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ndcX = (2.0 * screenX) / (double)width - 1.0;
    ndcY = 1.0 - (2.0 * screenY) / (double)height;
}

optional<Point> getNearestPoint(GLFWwindow* window, const set<Point>& clickedPoints) {
    double ndcX, ndcY;
    getCursorPositionInNDC(window, ndcX, ndcY);
    for (const auto& point : clickedPoints) {
        if (pointIsNearOtherPoint(point, Point(ndcX, ndcY))) {
            return point;
        }
    }
    return nullopt;
}

optional<Point> pointIsNearOtherPoints(const Point& point, const set<Point>& otherPoints) {
    for (const auto& other : otherPoints) {
        if (validClick(point, other)) {
            return other;
        }
    }
    return nullopt;
}

Point getNearestPointToTarget(const set<Point>& points, const Point& target) {
    Point nearestPoint;
    double minDistanceSq = std::numeric_limits<double>::max();
    for (const auto& point : points) {
        double distSq = point.distanceSquared(target);
        if (distSq < minDistanceSq) {
            minDistanceSq = distSq;
            nearestPoint = point;
        }
    }
    return nearestPoint;
}
