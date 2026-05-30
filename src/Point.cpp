#include "Point.h"

struct Point {
    double x;
    double y;

    static const double DEFAULT_X; // A value that is unlikely to be a valid point coordinate
    static const double DEFAULT_Y; // A value that is unlikely to be a

    // Optional: A constructor to make creating points easier
    Point(double x_val = DEFAULT_X, double y_val = DEFAULT_Y);
    
    // Optional: A method declaration
    void print() const;

    // overload equality operators for easy comparison
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    // Check if the point is at the default (0,0)
    bool isDefault() const { return x == DEFAULT_X && y == DEFAULT_Y; }
};
