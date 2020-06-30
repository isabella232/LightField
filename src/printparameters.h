#ifndef __PRINTPARAMETERS_H__
#define __PRINTPARAMETERS_H__

class PrintParameters {

public:

    PrintParameters() = default;
    PrintParameters(PrintParameters const&) = default;
    PrintParameters(PrintParameters&&) = default;
    PrintParameters& operator=(PrintParameters const&) = default;
    PrintParameters& operator=(PrintParameters&&) = default;

    //
    // Accessors
    //

    int layerExposureTime() const
    {
        return _layerExposureTime;
    }

    // unit: percent
    double powerLevel() const
    {

        return _powerLevel;
    }

    // unit: boolean (true/false)
    bool isPumpingEnabled() const
    {

        return _pumpingEnabled;
    }

    // unit: mm, multiples of 0.01
    double pumpUpDistance() const
    {

        return _pumpUpDistance;
    }

    // unit: mm/min
    double pumpUpVelocity_Effective() const
    {

        return _pumpUpVelocity;
    }

    // unit: ms
    int pumpUpPause() const
    {

        return _pumpUpPause;
    }

    // unit: mm, multiples of 0.01
    double pumpDownDistance_Effective() const
    {

        return _pumpUpDistance;
    }

    // unit: mm/min
    double pumpDownVelocity_Effective() const
    {

        return _pumpDownVelocity;
    }

    // unit: ms
    int pumpDownPause() const
    {

        return _pumpDownPause;
    }


    // unit: mm/min
    double noPumpUpVelocity() const
    {

        return _noPumpUpVelocity;
    }

    // unit: mm/min
    double noPumpDownVelocity_Effective() const
    {

        return _noPumpUpVelocity;
    }

    // unit: layers
    int pumpEveryNthLayer() const
    {

        return _pumpEveryNthLayer;
    }

    //unit: um
    int layerThickness() const
    {
        return _layerThickness;
    }

    //
    // Mutators
    //

    // unit: ms
    void setLayerExposureTime(int const value)
    {
        _layerExposureTime = value;
    }

    // unit: percent
    void setPowerLevel(double const value)
    {

        _powerLevel = value;
    }

    // unit: boolean (true/false)
    void setPumpingEnabled(bool const value)
    {

        _pumpingEnabled = value;
    }

    // unit: mm, multiples of 0.01
    void setPumpUpDistance(double const value)
    {

        _pumpUpDistance = value;
    }

    // unit: ms
    void setPumpUpPause(int const value)
    {

        _pumpUpPause = value;
    }

    // unit: ms
    void setPumpDownPause(int const value)
    {

        _pumpDownPause = value;
    }

    // unit: mm/min
    void setPumpUpVelocity(double const value)
    {

        _pumpUpVelocity = value;
    }
    // unit: mm/min
    void setPumpDownVelocity(double const value)
    {

        _pumpDownVelocity = value;
    }

    // unit: mm/min
    void setNoPumpUpVelocity(double const value)
    {

        _noPumpUpVelocity = value;
    }

    // unit: layer
    void setPumpEveryNthLayer(int const value)
    {

        _pumpEveryNthLayer = value;
    }

    //unit: um
    void setLayerThickness(int const value)
    {
        _layerThickness = value;
    }

private:

    bool _pumpingEnabled {false}; // boolean (true/false)
    double _pumpUpDistance {2.00}; // mm
    int _pumpUpPause {2000}; // ms
    int _pumpDownPause {4000}; // ms
    int _pumpUpVelocity {50}; // mm/min
    int _pumpDownVelocity {50}; // mm/min
    int _noPumpUpVelocity {200}; // mm/min
    int _pumpEveryNthLayer {1}; // layer
    int _layerExposureTime {1000}; //ms
    double _powerLevel {50.0}; // percent
    int _layerThickness {50}; // um

};

#endif // !__PRINTPARAMETERS_H__
