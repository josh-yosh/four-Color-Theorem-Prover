#include "Point.h"
#include <iostream>

// Implement the constructor
Point::Point(float x_val, float y_val) : x(x_val), y(y_val) {}

// Implement the print method
void Point::print() const {
    std::cout << "Point(" << x << ", " << y << ")\n";
}

// Implement equality operators
bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y;
}

bool Point::operator!=(const Point& other) const {
    return !(*this == other);
}

// Implement the default check
bool Point::isDefault() const { 
    return x == 0.0 && y == 0.0; 
}