// Point.h
#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <functional>
#include <tuple>
#include <set>

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
    
    double distanceSquared(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return dx * dx + dy * dy;
    }
};

namespace std {
    template <>
    struct hash<Point> {
        size_t operator()(const Point& p) const {
            // A classic bit-shifting technique to combine two hashes
            size_t h1 = hash<int>{}(p.x);
            size_t h2 = hash<int>{}(p.y);
            return h1 ^ (h2 << 1); 
        }
    };
}

struct SetPointHash {
    size_t operator()(const std::set<Point>& pointSet) const {
        size_t seed = 0;
        for (const auto& point : pointSet) {
            size_t nextHash = std::hash<Point>{}(point);
            // The hash_combine formula
            seed ^= nextHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

struct NestedSetPointHash {
    size_t operator()(const std::set<std::set<Point>>& nestedSet) const {
        size_t seed = 0;
        SetPointHash innerHasher;
        for (const auto& innerSet : nestedSet) {
            size_t nextHash = innerHasher(innerSet);
            seed ^= nextHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
#endif