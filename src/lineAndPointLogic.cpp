#include <unordered_map>
#include <optional>
#include <set>
#include <vector>
#include "lineAndPointLogic.h"
#include "Point.h"
using namespace std;

constexpr double CLICK_THRESHOLD = 0.025;


optional<Point> getIntersectionPoint(const set<Point>& line1, const set<Point>& line2) {
    // Convert sets to vectors for easier access
    Point p1, p2, p3, p4;
    getPointsFromLine(line1, line2, p1, p2, p3, p4);
    float a1, b1, c1, a2, b2, c2, determinant;
    getConstantsFromLine(line1, line2, a1, b1, c1, a2, b2, c2, determinant);


    bool collinear = (determinant == 0);
    bool intersectionExists = false;
    float x, y;
    if (!collinear){
        x = (b2 * c1 - b1 * c2) / determinant;
        y = (a1 * c2 - a2 * c1) / determinant;
        Point intersectionPoint{x, y};
        bool intersectionPointNotEndpoint = !nearlyEqual(intersectionPoint, p1) && !nearlyEqual(intersectionPoint, p2) && !nearlyEqual(intersectionPoint, p3) && !nearlyEqual(intersectionPoint, p4);
        intersectionExists = pointWithinSegments(intersectionPoint, p1, p2) && pointWithinSegments(intersectionPoint, p3, p4) && intersectionPointNotEndpoint;
    }
     
    if (intersectionExists) {
        return Point{x, y};
    } else {
        return std::nullopt; // Lines are parallel or coincident, no single intersection point
    }
}

void splitLineIntoSegments(const set<Point>& line, Point intersectionPoint, set<set<Point>>& allEdges) {
    vector<Point> points(line.begin(), line.end());
    Point p1 = points[0];
    Point p2 = points[1];

    if (nearlyEqual(p1, intersectionPoint) || nearlyEqual(p2, intersectionPoint)) {
        // Intersection is at an endpoint — no split needed, original line stands
        return;
    }

    // Create two new segments: (p1, intersection) and (intersection, p2)
    set<Point> segment1 = {p1, intersectionPoint};
    set<Point> segment2 = {intersectionPoint, p2};

    // Add the new segments to the collection of all edges
    allEdges.insert(segment1);
    allEdges.insert(segment2);
}

void breakLineIntoSegments(const set<Point>& line, set<Point> intersectionPoints, set<set<Point>>& allEdges) {
    if (line.empty()) return;

    vector<Point> pointsOnLine(intersectionPoints.begin(), intersectionPoints.end());
    pointsOnLine.insert(pointsOnLine.end(), line.begin(), line.end());

    cout << "points on line before sorting: " << pointsOnLine.size() << "\n";
    Point p1 = *line.begin();

    sort(pointsOnLine.begin(), pointsOnLine.end(), [&](const Point& a, const Point& b) {
        float da = (a.x - p1.x) * (a.x - p1.x) + (a.y - p1.y) * (a.y - p1.y);
        float db = (b.x - p1.x) * (b.x - p1.x) + (b.y - p1.y) * (b.y - p1.y);
        return da < db;
    }); 

    pointsOnLine.erase(std::unique(pointsOnLine.begin(), pointsOnLine.end()), pointsOnLine.end());

    // Create segments between consecutive points
    for(int i = 0; i < pointsOnLine.size() - 1; i++) {
        set<Point> segment = {pointsOnLine[i], pointsOnLine[i + 1]};
        allEdges.insert(segment);
    }
}

bool isPointOnLineSegment(const Point& p, const set<Point>& line) {
    vector<Point> points(line.begin(), line.end());
    Point p1 = points[0];
    Point p2 = points[1];

    // Vector from p1 to p2 using float
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float epsilon = 0.025f; // Tolerance for "closeness" to the line segment
    
    // Length squared of the line segment
    float segmentLengthSq = dx * dx + dy * dy;
    
    // If p1 and p2 are the exact same point
    if (segmentLengthSq == 0.0f) {
        if (std::sqrt(p.distanceSquared(p1)) <= epsilon) {
            return false;
        }
        return false;
    }
    
    // Project point p onto the line segment
    float t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / segmentLengthSq;
    
    // Clamp t to the range [0.0f, 1.0f]
    t = std::clamp(t, 0.0f, 1.0f);
    
    // Calculate the closest point on the segment
    Point closestPoint{
        p1.x + t * dx,
        p1.y + t * dy
    };
    
    // Check if the distance to the closest point is within our tolerance
    float distSq = p.distanceSquared(closestPoint);
    bool lessThanEpsilon = distSq <= epsilon * epsilon;
    bool withinSegmentBounds = pointWithinSegments(closestPoint, p1, p2);
    bool pointIsNotEndpoint = !nearlyEqual(closestPoint, p1) && !nearlyEqual(closestPoint, p2);
    if(lessThanEpsilon && withinSegmentBounds && pointIsNotEndpoint){
        cout << "closest point: (" << closestPoint.x << ", " << closestPoint.y << ")\n";
        cout << "p1: (" << p1.x << ", " << p1.y << "), p2: (" << p2.x << ", " << p2.y << ")\n";
    }
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

void getPointsFromLine(const set<Point>& line1, const set<Point>& line2, Point& p1, Point& p2, Point& p3, Point& p4) {
    vector<Point> points1(line1.begin(), line1.end());
    vector<Point> points2(line2.begin(), line2.end());
    p1 = points1[0];
    p2 = points1[1];
    p3 = points2[0];
    p4 = points2[1];
}

void getConstantsFromLine(const set<Point>& line1, const set<Point>& line2, float& a1, float& b1, float& c1, float& a2, float& b2, float& c2, float& determinant) {
    vector<Point> points1(line1.begin(), line1.end());
    vector<Point> points2(line2.begin(), line2.end());

    Point p1 = points1[0];
    Point p2 = points1[1];
    Point p3 = points2[0];
    Point p4 = points2[1];

    // Line 1 represented as a1x + b1y = c1
    a1 = p2.y - p1.y;
    b1 = p1.x - p2.x;
    c1 = a1 * p1.x + b1 * p1.y;

    // Line 2 represented as a2x + b2y = c2
    a2 = p4.y - p3.y;
    b2 = p3.x - p4.x;
    c2 = a2 * p3.x + b2 * p3.y;

    determinant = a1 * b2 - a2 * b1;
}

optional<Point> closestPointOnLine(const Point& p, const Point& p1, const Point& p2) {
    // Vector from p1 to p2 using float
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float epsilon = 0.05f; // Tolerance for "closeness" to the line segment
    
    // Length squared of the line segment
    float segmentLengthSq = dx * dx + dy * dy;
    
    // If p1 and p2 are the exact same point
    if (segmentLengthSq == 0.0f) {
        if (std::sqrt(p.distanceSquared(p1)) <= epsilon) {
            return p1;
        }
        return std::nullopt;
    }
    
    // Project point p onto the line segment
    float t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / segmentLengthSq;
    
    // Clamp t to the range [0.0f, 1.0f]
    t = std::clamp(t, 0.0f, 1.0f);
    
    // Calculate the closest point on the segment
    // This will now compile perfectly with your float-based Point struct
    Point closestPoint{
        p1.x + t * dx,
        p1.y + t * dy
    };
    
    // Check if the distance to the closest point is within our tolerance
    float distSq = p.distanceSquared(closestPoint);
    if (distSq <= epsilon * epsilon) {
        cout << "Point is close enough to the line segment. Closest point: (" << closestPoint.x << ", " << closestPoint.y << ")\n";
        return closestPoint;
    }
    
    return std::nullopt;
}

bool hasActiveConnection(const set<Point>& activeConnections) {
    return !activeConnections.empty();
}

// determines if a click is close enough to a point to be considered a valid selection
bool validClick(const Point& point, const Point& click) {
    double distanceSquared = (point.x - click.x) * (point.x - click.x) + (point.y - click.y) * (point.y - click.y);
    return distanceSquared < (CLICK_THRESHOLD * CLICK_THRESHOLD); // Compare squared distances to avoid sqrt
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
    glfwGetWindowSize(window, &width, &height); // Must be WindowSize to match CursorPos!

    // Calculate NDC (-1.0 to 1.0)
    ndcX = (2.0 * screenX) / (double)width - 1.0;
    ndcY = 1.0 - (2.0 * screenY) / (double)height;    
}

// Returns the first point that is near the click position, or nullopt if none are close enough
optional<Point> getNearestPoint(GLFWwindow* window, const set<Point>& clickedPoints) {
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

optional<Point> pointIsNearOtherPoints(const Point& point, const set<Point>& otherPoints) {
    for (const auto& other : otherPoints) {
        if (validClick(point, other)) {
            return other; // Found a nearby point
        }
    }
    return nullopt; // No nearby points found
}

