// Point.h
#ifndef POINT_H
#define POINT_H

struct Point {
    double x; // Variable declared
    double y; // Variable declared

    // Constructor definition
    Point(double x_val = 0.0, double y_val = 0.0);
    
    void print() const; 
};

#endif