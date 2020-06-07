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
        _buildPlatformOffset = profile._buildPlatformOffset;
        _disregardFirstLayerHeight = profile._disregardFirstLayerHeight;
        _baseLayerCount = profile._baseLayerCount;
        _buildPlatformOffset = profile._buildPlatformOffset;
        _baseLayerParameters = profile._baseLayerParameters;
        _bodyLayerParameters = profile._bodyLayerParameters;
    }

    virtual ~PrintProfile() override
    {
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

    const QString& profileName() const
    {
        return _name;
    }

    bool isDefault() const
    {
        return _default;
    }

    bool isActive() const
    {
        return _active;
    }

    int buildPlatformOffset() const
    {
        return _buildPlatformOffset;
    }

    bool disregardFirstLayerHeight() const
    {
        return _disregardFirstLayerHeight;
    }

    int heatingTemperature() const
    {
        return _heatingTemperature;
    }

    int baseLayerCount() const
    {
        return _baseLayerCount;
    }

    PrintParameters& baseLayerParameters()
    {
        return _baseLayerParameters;
    }

    const PrintParameters& baseLayerParameters() const
    {
        return _baseLayerParameters;
    }

    PrintParameters& bodyLayerParameters()
    {
        return _bodyLayerParameters;
    }

    const PrintParameters& bodyLayerParameters() const
    {
        return _bodyLayerParameters;
    }

    void setProfileName(const QString& newName)
    {
        emit profileNameChanged(newName);
        _name = newName;
    }

    void setDefault(bool isDefault)
    {
        _default = isDefault;
    }

    void setActive(bool isActive)
    {
        _active = isActive;
    }

    void setBuildPlatformOffset(int newOffset)
    {
        _buildPlatformOffset = newOffset;
    }

    void setDisregardFirstLayerHeight(bool value)
    {
        _disregardFirstLayerHeight = value;
    }

    void setHeatingTemperature(int temperature)
    {
        _heatingTemperature = temperature;
    }

    void setBaseLayerCount(int newCount)
    {
        _baseLayerCount = newCount;
    }

    void setBaseLayerParameters(const PrintParameters& newParameters)
    {
        _baseLayerParameters = newParameters;
    }

    void setBodyLayerParameters(const PrintParameters& newParameters)
    {
        _bodyLayerParameters = newParameters;
    }

private:

    QString _name;
    bool _default;
    bool _active;
    int _baseLayerCount;
    int _buildPlatformOffset;
    bool _disregardFirstLayerHeight;
    int _heatingTemperature;
    PrintParameters _baseLayerParameters;
    PrintParameters _bodyLayerParameters;
    bool _defaultProfile;

signals:
    void profileNameChanged(const QString& newName);
};

#endif // !__PRINTPROFILE_H__
