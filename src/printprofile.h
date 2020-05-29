#ifndef __PRINTPROFILE_H__
#define __PRINTPROFILE_H__

#include <QtCore>
#include "debug.h"
#include "printparameters.h"

class PrintProfile: public QObject
{
    Q_OBJECT

public:

    PrintProfile(QObject* parent = nullptr):
        QObject(parent)
    {
    }

    PrintProfile(const PrintProfile& profile):
        QObject(profile.parent())
    {
        _name = profile._name;
        _baseLayerCount = profile._baseLayerCount;
        _baseLayerParameters = profile._baseLayerParameters;
        _bodyLayerParameters = profile._bodyLayerParameters;
    }

    virtual ~PrintProfile( ) override {
        /*empty*/
    }

    void debugPrint()
    {
        debug("#############################################\n");
        debug("PrintProfile %s: \n", _name.toUtf8( ).data( ));
        debug("#############################################\n");
        debug("     objectAddress:                  %d\n", this);
        debug("     baseLayerCount                  %d\n", _baseLayerCount);
        //debug("     baseLayersParametersEnabled        %d\n", _baseLayersParametersEnabled);
        debug("     baseLayersParameters:    \n");
        debug("     objectAddress:                  %d\n", &_baseLayerParameters);
        debug("         powerLevel:                 %d\n", _baseLayerParameters.powerLevel());
        debug("         pumpUpVelocity:             %d\n", _baseLayerParameters.pumpUpVelocity_Effective());
        debug("         pumpUpPause:                %d\n", _baseLayerParameters.pumpUpPause());
        debug("         pumpDownVelocity:           %d\n", _baseLayerParameters.pumpDownVelocity_Effective());
        debug("         pumpDownPause:              %d\n", _baseLayerParameters.pumpDownPause());
        debug("         layerThickness:             %d\n", _baseLayerParameters.layerThickness());
        debug("         pumpUpDistance:             %d\n", _baseLayerParameters.pumpUpDistance());
        debug("         noPumpUpVelocity:           %d\n", _baseLayerParameters.noPumpUpVelocity());
        debug("         layerExposureTime:          %d\n", _baseLayerParameters.layerExposureTime());
        debug("         pumpEveryNthLayer:          %d\n", _baseLayerParameters.pumpEveryNthLayer());
        /*debug("         pumpDownTime_Effective:     %d\n", _baseLayersPumpingParameters.pumpDownTime_Effective());
        debug("         pumpUpVelocity_Effective:   %d\n", _baseLayersPumpingParameters.pumpUpVelocity_Effective());
        debug("         pumpDownDistance_Effective: %d\n", _baseLayersPumpingParameters.pumpDownDistance_Effective());
        debug("         pumpDownVelocity_Effective: %d\n", _baseLayersPumpingParameters.pumpDownVelocity_Effective());*/
        //debug("     bodyLayersParametersEnabled        %d\n", _bodyLayersParametersEnabled);
        debug("     bodyLayersParameters:    \n");
        debug("     objectAddress:                  %d\n", &_bodyLayerParameters);
        debug("         powerLevel:                 %d\n", _bodyLayerParameters.powerLevel());
        debug("         pumpUpVelocity:             %d\n", _bodyLayerParameters.pumpUpVelocity_Effective());
        debug("         pumpUpPause:                %d\n", _bodyLayerParameters.pumpUpPause());
        debug("         pumpDownVelocity:           %d\n", _bodyLayerParameters.pumpDownVelocity_Effective());
        debug("         pumpDownPause:              %d\n", _bodyLayerParameters.pumpDownPause());
        debug("         layerThickness:             %d\n", _bodyLayerParameters.layerThickness());
        debug("         pumpUpDistance:             %d\n", _bodyLayerParameters.pumpUpDistance());
        debug("         noPumpUpVelocity:           %d\n", _bodyLayerParameters.noPumpUpVelocity());
        debug("         layerExposureTime:          %d\n", _bodyLayerParameters.layerExposureTime());
        debug("         pumpEveryNthLayer:          %d\n", _bodyLayerParameters.pumpEveryNthLayer());
        /*debug("         pumpDownTime_Effective:     %d\n", _bodyLayersPumpingParameters.pumpDownTime_Effective());
        debug("         pumpUpVelocity_Effective:   %d\n", _bodyLayersPumpingParameters.pumpUpVelocity_Effective());
        debug("         pumpDownDistance_Effective: %d\n", _bodyLayersPumpingParameters.pumpDownDistance_Effective());
        debug("         pumpDownVelocity_Effective: %d\n", _bodyLayersPumpingParameters.pumpDownVelocity_Effective());*/
        debug("#############################################\n");
    }

    //
    // Accessors
    //

    QString const& profileName( ) const {
        return _name;
    }

    bool isDefault() {
        return _default;
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
        emit profileNameChanged( newName );
        _name = newName;
    }

    void setDefault(bool isDefault) {
        _default = isDefault;
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
    bool            _default;
    int             _baseLayerCount       { };
    PrintParameters _baseLayerParameters;
    PrintParameters _bodyLayerParameters;
    bool            _defaultProfile;    

signals:
    void profileNameChanged( QString const& newName );

public slots:

protected slots:

private slots:

};

#endif // !__PRINTPROFILE_H__
