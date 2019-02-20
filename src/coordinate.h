#ifndef __COORDINATE_H__
#define __COORDINATE_H__

class Coordinate {

public:

    Coordinate( float const min_ = 0.0f, float const max_ = 0.0f ): min( min_ ), max( max_ ), size( max_ - min_ ) {
        /*empty*/
    }

    float min;
    float max;
    float size;

};

#endif // __COORDINATE_H__