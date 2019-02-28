#ifndef __COORDINATE_H__
#define __COORDINATE_H__

class Coordinate {

public:

    constexpr Coordinate( ): min( 0.0f ), max( 0.0f ), size( 0.0f ) {
        /*empty*/
    }

    constexpr Coordinate( float const min_, float const max_ ): min( min_ ), max( max_ ), size( max_ - min_ ) {
        /*empty*/
    }

    float min;
    float max;
    float size;

};

#endif // __COORDINATE_H__