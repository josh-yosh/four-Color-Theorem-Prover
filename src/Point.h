// Point.h
#ifndef POINT_H
#define POINT_H

struct Point {
    double x; // Variable declared
    double y; // Variable declared

    // Constructor definition
    Point(double x_val = 0.0, double y_val = 0.0);
    
    void print() const; 

    // overload equality operators for easy comparison
    bool operator==(const Point& other) const;
    bool operator!=(const Point& other) const;

    // Check if the point is at the default (0,0)
    bool isDefault() const;
};

#endif