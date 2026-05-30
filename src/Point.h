// Point.h
#ifndef POINT_H
#define POINT_H

#include <iostream>

struct Point {
    float x; // Variable declared
    float y; // Variable declared

    // Constructor definition
    Point(float x_val = 0.0, float y_val = 0.0);
    
    void print() const; 

    // overload equality operators for easy comparison
    bool operator==(const Point& other) const;
    bool operator!=(const Point& other) const;

    // Check if the point is at the default (0,0)
    bool isDefault() const;

    // Overload less-than operator so std::set can sort and store Points
    bool operator<(const Point& other) const {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }
};

#endif