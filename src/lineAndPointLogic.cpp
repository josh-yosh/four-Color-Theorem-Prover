#include <unordered_map>
#include <optional>
#include <set>
#include <vector>
#include "lineAndPointLogic.h"
#include "Point.h"
using namespace std;

constexpr double CLICK_THRESHOLD = 0.05;

void getIntersectionPoint(const set<Point>& line1, const set<Point>& line2, unordered_map<set<set<Point>>, optional<Point>, NestedSetPointHash>& intersectionPoints, vector<Point>& clickedPoints, set<set<Point>>& allConnections) {
    // Convert sets to vectors for easier access
    Point p1, p2, p3, p4;
    getPointsFromLine(line1, line2, p1, p2, p3, p4);
    float a1, b1, c1, a2, b2, c2, determinant;
    getConstantsFromLine(line1, line2, a1, b1, c1, a2, b2, c2, determinant);

    bool collinear = (determinant == 0);

    if (collinear) {
        intersectionPoints[{line1, line2}] = nullopt; // Lines are parallel, no intersection
    } else {
        splitLineIntoSegments(line1, line2, intersectionPoints, clickedPoints, allConnections);
    }
}

void splitLineIntoSegments(const set<Point>& line1, const set<Point>& line2, unordered_map<set<set<Point>>, optional<Point>, NestedSetPointHash>& intersectionPoints, vector<Point>& clickedPoints, set<set<Point>>& allConnections) {
    Point p1, p2, p3, p4;
    getPointsFromLine(line1, line2, p1, p2, p3, p4);
    float a1, b1, c1, a2, b2, c2, determinant;
    getConstantsFromLine(line1, line2, a1, b1, c1, a2, b2, c2, determinant);
    float x = (b2 * c1 - b1 * c2) / determinant;
    float y = (a1 * c2 - a2 * c1) / determinant;

    if (pointWithinSegments(Point(x, y), p1, p2) && pointWithinSegments(Point(x, y), p3, p4)) {
            Point intersection(x, y);
            intersectionPoints[{line1, line2}] = intersection; // Store the intersection point
            bool intersectionTooCloseToExistingPoint = false;

            for (const auto& existingPoint : clickedPoints) {
                double distanceSquared = (existingPoint.x - intersection.x) * (existingPoint.x - intersection.x) + (existingPoint.y - intersection.y) * (existingPoint.y - intersection.y);
                if (distanceSquared < (CLICK_THRESHOLD * CLICK_THRESHOLD)) {
                    intersectionTooCloseToExistingPoint = true;
                    break;
                }
            }
            if (!intersectionTooCloseToExistingPoint) {
                clickedPoints.push_back(intersection); // Add the intersection point to the list of points to be drawn
                allConnections.erase(line1); // Remove the original lines that intersected
                allConnections.erase(line2);
                allConnections.insert({p1, intersection}); // Add the new lines formed by the intersection
                allConnections.insert({intersection, p2});
                allConnections.insert({p3, intersection});
                allConnections.insert({intersection, p4});
            }
        }
}

bool pointWithinSegments(const Point& p, const Point& p1, const Point& p2) {
    return (min(p1.x, p2.x) <= p.x && p.x <= max(p1.x, p2.x)) &&
           (min(p1.y, p2.y) <= p.y && p.y <= max(p1.y, p2.y));
}
void getPointsFromLine(const set<Point>& line1, const set<Point>& line2, Point& p1, Point& p2, Point& p3, Point& p4) {
    vector<Point> points(line1.begin(), line1.end());
    vector<Point> points2(line2.begin(), line2.end());
    p1 = points[0];
    p2 = points[1];
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

optional<Point> pointOnLineSegment(const Point& p, const Point& p1, const Point& p2) {
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