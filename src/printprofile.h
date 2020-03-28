#ifndef __PRINTPROFILE_H__
#define __PRINTPROFILE_H__

#include "printpumpingparameters.h"

class PrintProfile: public QObject {

    Q_OBJECT;

public:

    PrintProfile( QObject* parent = nullptr ): QObject( parent ) {
        /*empty*/
    }

    virtual ~PrintProfile( ) override {
        /*empty*/
    }

    PrintProfile( PrintProfile const& profile ): QObject( profile.parent( ) ) {
        _name                     = profile._name;

        _baseLayerCount           = profile._baseLayerCount;
        _baseLayersPumpingEnabled = profile._baseLayersPumpingEnabled;
        _baseLayersParameters     = profile._baseLayersParameters;

        _bodyLayersPumpingEnabled = profile._bodyLayersPumpingEnabled;
        _bodyLayersParameters     = profile._bodyLayersParameters;
    }

    //
    // Accessors
    //

    QString const& profileName( ) const {
        return _name;
    }

    int baseLayerCount( ) const {
        return _baseLayerCount;
    }

    bool baseLayersPumpingEnabled() const {
        return _baseLayersPumpingEnabled;
    }

    PrintPumpingParameters& baseLayersParameters( ) {
        return _baseLayersParameters;
    }

    PrintPumpingParameters const& baseLayersParameters( ) const {
        return _baseLayersParameters;
    }

    bool bodyLayersPumpingEnabled() const {
        return _bodyLayersPumpingEnabled;
    }

    PrintPumpingParameters& bodyLayersParameters( ) {
        return _bodyLayersParameters;
    }

    PrintPumpingParameters const& bodyLayersParameters( ) const {
        return _bodyLayersParameters;
    }

    //
    // Mutators
    //

    void setProfileName( QString const& newName ) {
        _name = newName;
    }

    void setBaseLayerCount( int const newCount ) {
        _baseLayerCount = newCount;
    }

    void setBaseLayersPumpingParameters( PrintPumpingParameters const& newParameters ) {
        _baseLayersParameters = newParameters;
    }

    void setBodyLayersPumpingParameters( PrintPumpingParameters const& newParameters ) {
        _bodyLayersParameters = newParameters;
    }

    void setBaseLayersPumpingEnabled(bool value) {
        _baseLayersPumpingEnabled = value;
    }

    void setBodyLayersPumpingEnabled(bool value) {
        _bodyLayersPumpingEnabled = value;
    }

protected:

private:

    QString                _name;

    int                    _baseLayerCount              { };
    bool                   _baseLayersPumpingEnabled    { };
    PrintPumpingParameters _baseLayersParameters;

    bool                   _bodyLayersPumpingEnabled    { };
    PrintPumpingParameters _bodyLayersParameters;

};

#endif // !__PRINTPROFILE_H__
