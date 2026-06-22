#include "Point.h"
#include <iostream>
#include <cmath>

bool nearlyEqual(const Point& a, const Point& b) {
    constexpr float eps = 1e-3f;
    return fabs(a.x - b.x) < eps && fabs(a.y - b.y) < eps;
}

// Implement the constructor
Point::Point(float x_val, float y_val) : x(x_val), y(y_val) {}

// Implement the print method
void Point::print() const {
    std::cout << "Point(" << x << ", " << y << ")\n";
}

// Implement equality operators
bool Point::operator==(const Point& other) const {
    return nearlyEqual(*this, other);
}

bool Point::operator!=(const Point& other) const {
    return !nearlyEqual(*this, other);
}

// Implement the default check
bool Point::isDefault() const { 
    return nearlyEqual(*this, Point(0.0, 0.0)); 
}