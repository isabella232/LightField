#ifndef __COORDINATE_H__
#define __COORDINATE_H__

class Coordinate {

public:

    constexpr Coordinate(): min(0.0), max(0.0), size(0.0) {}

    constexpr Coordinate(double min_, double max_): min(min_), max(max_), size(max_ - min_) {}

    double min;
    double max;
    double size;

};

#endif // __COORDINATE_H__
