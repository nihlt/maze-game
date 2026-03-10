#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cmath>

inline double lerp(double a, double b, double t) { return a + (b - a) * t; }

struct Vec2 {
    double x, y;
    Vec2() : x(0), y(0) {}
    Vec2(double _x, double _y) : x(_x), y(_y) {}
};

inline double angleWrap(double a) {
    while (a >= 360.0) a -= 360.0;
    while (a <  0.0)   a += 360.0;
    return a;
}

#endif // MATH_UTILS_H
