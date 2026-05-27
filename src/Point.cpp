#ifndef POINT_H
#define POINT_H

struct Point {
    double x;
    double y;

    // Optional: A constructor to make creating points easier
    Point(double x_val = 0.0, double y_val = 0.0);
    
    // Optional: A method declaration
    void print() const;
};

#endif // POINT_H