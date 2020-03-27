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
        _name                        = profile._name;

        _baseLayerCount              = profile._baseLayerCount;
        _baseLayersPumpingEnabled    = profile._baseLayersPumpingEnabled;
        _baseLayersPumpingParameters = profile._baseLayersPumpingParameters;

        _bodyLayersPumpingEnabled    = profile._bodyLayersPumpingEnabled;
        _bodyLayersPumpingParameters = profile._bodyLayersPumpingParameters;
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

    PrintPumpingParameters& baseLayersPumpingParameters( ) {
        return _baseLayersPumpingParameters;
    }

    PrintPumpingParameters const& baseLayersPumpingParameters( ) const {
        return _baseLayersPumpingParameters;
    }

    bool bodyLayersPumpingEnabled() const {
        return _bodyLayersPumpingEnabled;
    }

    PrintPumpingParameters& bodyLayersPumpingParameters( ) {
        return _bodyLayersPumpingParameters;
    }

    PrintPumpingParameters const& bodyLayersPumpingParameters( ) const {
        return _bodyLayersPumpingParameters;
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
        _baseLayersPumpingParameters = newParameters;
    }

    void setBodyLayersPumpingParameters( PrintPumpingParameters const& newParameters ) {
        _bodyLayersPumpingParameters = newParameters;
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
    PrintPumpingParameters _baseLayersPumpingParameters;

    bool                   _bodyLayersPumpingEnabled    { };
    PrintPumpingParameters _bodyLayersPumpingParameters;

};

#endif // !__PRINTPROFILE_H__
