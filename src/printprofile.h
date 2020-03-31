#ifndef __PRINTPROFILE_H__
#define __PRINTPROFILE_H__

#include "printparameters.h"

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
        _name                 = profile._name;

        _baseLayerCount       = profile._baseLayerCount;
        _baseLayerParameters = profile._baseLayerParameters;

        _bodyLayerParameters = profile._bodyLayerParameters;
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

    PrintParameters& baseLayerParameters( ) {
        return _baseLayerParameters;
    }

    PrintParameters const& baseLayerParameters( ) const {
        return _baseLayerParameters;
    }

    PrintParameters& bodyLayerParameters( ) {
        return _bodyLayerParameters;
    }

    PrintParameters const& bodyLayerParameters( ) const {
        return _bodyLayerParameters;
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

    void setBaseLayerParameters( PrintParameters const& newParameters ) {
        _baseLayerParameters = newParameters;
    }

    void setBodyLayerParameters( PrintParameters const& newParameters ) {
        _bodyLayerParameters = newParameters;
    }

protected:

private:

    QString         _name;

    int             _baseLayerCount       { };
    PrintParameters _baseLayerParameters;

    PrintParameters _bodyLayerParameters;

};

#endif // !__PRINTPROFILE_H__
