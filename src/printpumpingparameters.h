#ifndef __PRINTPUMPINGPARAMETERS_H__
#define __PRINTPUMPINGPARAMETERS_H__

class PrintPumpingParameters {

public:

    PrintPumpingParameters( )                                          = default;
    PrintPumpingParameters( PrintPumpingParameters const& )            = default;
    PrintPumpingParameters( PrintPumpingParameters&& )                 = default;
    PrintPumpingParameters& operator=( PrintPumpingParameters const& ) = default;
    PrintPumpingParameters& operator=( PrintPumpingParameters&& )      = default;

    //
    // Accessors
    //

    // unit: mm, multiples of 0.01
    double pumpUpDistance( ) const {
        return _pumpUpDistance;
    }

    // unit: ms
    int pumpUpTime( ) const {
        return _pumpUpTime;
    }

    // unit: mm/min
    double pumpUpVelocity_Effective( ) const {
        return _pumpUpDistance / ( _pumpUpTime / 1000.0 / 60.0 );
    }

    // unit: ms
    int pumpUpPause( ) const {
        return _pumpUpPause;
    }

    // unit: ms
    int pumpDownTime( ) const {
        return _pumpDownTime;
    }

    // unit: ms
    int pumpDownPause( ) const {
        return _pumpDownPause;
    }

    // unit: mm/min
    double noPumpUpVelocity( ) const {
        return _noPumpUpVelocity;
    }

    // unit: µm
    int layerThickness( ) const {
        return _layerThickness;
    }

    // unit: ms
    int layerExposureTime( ) const {
        return _layerExposureTime;
    }

    // unit: none
    int pumpEveryNthLayer( ) const {
        return _pumpEveryNthLayer;
    }

    //
    // Mutators
    //

    // unit: mm, multiples of 0.01
    void setPumpUpDistance( double const value ) {
        _pumpUpDistance = value;
    }

    // unit: ms
    void setPumpUpTime( int const value ) {
        _pumpUpTime = value;
    }

    // unit: ms
    void setPumpUpPause( int const value ) {
        _pumpUpPause = value;
    }

    // unit: ms
    void setPumpDownTime( int const value ) {
        _pumpDownTime = value;
    }

    // unit: ms
    void setPumpDownPause( int const value ) {
        _pumpDownPause = value;
    }

    // unit: mm/min
    void setNoPumpUpVelocity( double const value ) {
        _noPumpUpVelocity = value;
    }

    // unit: µm
    void setLayerThickness( int const value ) {
        _layerThickness = value;
    }

    // unit: ms
    void setLayerExposureTime( int const value ) {
        _layerExposureTime = value;
    }

    // unit: none
    void setPumpEveryNthLayer( int const value ) {
        _pumpEveryNthLayer = value;
    }

private:

    double _pumpUpDistance    { };
    int    _pumpUpTime        { };
    int    _pumpUpPause       { };
    int    _pumpDownTime      { };
    int    _pumpDownPause     { };
    int    _noPumpUpVelocity  { };
    int    _layerThickness    { };
    int    _layerExposureTime { };
    int    _pumpEveryNthLayer { };

};

#endif //!__PRINTPUMPINGPARAMETERS_H__
