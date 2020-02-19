#ifndef __PRINTPROFILE_H__
#define __PRINTPROFILE_H__

#include "printpumpingparameters.h"

class PrintProfile: public QObject {

    Q_OBJECT;

public:

    PrintProfile( QObject* parent = nullptr ): QObject( parent ) {

    }

    virtual ~PrintProfile( ) override {
        /*empty*/
    }

    PrintProfile* copy() {
        PrintProfile* printProfile = new PrintProfile();

        printProfile->_name = this->_name;
        printProfile->_baseLayerCount = this->_baseLayerCount;
        printProfile->_baseLayersPumpingEnabled = this->_baseLayersPumpingEnabled;
        printProfile->_bodyLayersPumpingEnabled = this->_bodyLayersPumpingEnabled;

        printProfile->_baseLayersPumpingParameters = this->_baseLayersPumpingParameters;
        printProfile->_bodyLayersPumpingParameters = this->_bodyLayersPumpingParameters;

        return printProfile;
    }

    void debugPrint()
    {
        debug("#############################################\n");
        debug("PrintProfile %s: \n", _name.toUtf8( ).data( ));
        debug("#############################################\n");
        debug("     objectAddress:                  %d\n", this);
        debug("     baseLayerCount                  %d\n", _baseLayerCount);
        debug("     baseLayersPumpingEnabled        %d\n", _baseLayersPumpingEnabled);
        debug("     baseLayersPumpingParameters:    \n");
        debug("     objectAddress:                  %d\n", &_baseLayersPumpingParameters);
        debug("         powerLevel:                 %d\n", _baseLayersPumpingParameters.powerLevel());
        debug("         pumpUpTime:                 %d\n", _baseLayersPumpingParameters.pumpUpTime());
        debug("         pumpUpPause:                %d\n", _baseLayersPumpingParameters.pumpUpPause());
        debug("         pumpDownPause:              %d\n", _baseLayersPumpingParameters.pumpDownPause());
        debug("         layerThickness:             %d\n", _baseLayersPumpingParameters.layerThickness());
        debug("         pumpUpDistance:             %d\n", _baseLayersPumpingParameters.pumpUpDistance());
        debug("         noPumpUpVelocity:           %d\n", _baseLayersPumpingParameters.noPumpUpVelocity());
        debug("         layerExposureTime:          %d\n", _baseLayersPumpingParameters.layerExposureTime());
        debug("         pumpEveryNthLayer:          %d\n", _baseLayersPumpingParameters.pumpEveryNthLayer());
        /*debug("         pumpDownTime_Effective:     %d\n", _baseLayersPumpingParameters.pumpDownTime_Effective());
        debug("         pumpUpVelocity_Effective:   %d\n", _baseLayersPumpingParameters.pumpUpVelocity_Effective());
        debug("         pumpDownDistance_Effective: %d\n", _baseLayersPumpingParameters.pumpDownDistance_Effective());
        debug("         pumpDownVelocity_Effective: %d\n", _baseLayersPumpingParameters.pumpDownVelocity_Effective());*/
        debug("     bodyLayersPumpingEnabled        %d\n", _bodyLayersPumpingEnabled);
        debug("     bodyLayersPumpingParameters:    \n");
        debug("     objectAddress:                  %d\n", &_bodyLayersPumpingParameters);
        debug("         powerLevel:                 %d\n", _bodyLayersPumpingParameters.powerLevel());
        debug("         pumpUpTime:                 %d\n", _bodyLayersPumpingParameters.pumpUpTime());
        debug("         pumpUpPause:                %d\n", _bodyLayersPumpingParameters.pumpUpPause());
        debug("         pumpDownPause:              %d\n", _bodyLayersPumpingParameters.pumpDownPause());
        debug("         layerThickness:             %d\n", _bodyLayersPumpingParameters.layerThickness());
        debug("         pumpUpDistance:             %d\n", _bodyLayersPumpingParameters.pumpUpDistance());
        debug("         noPumpUpVelocity:           %d\n", _bodyLayersPumpingParameters.noPumpUpVelocity());
        debug("         layerExposureTime:          %d\n", _bodyLayersPumpingParameters.layerExposureTime());
        debug("         pumpEveryNthLayer:          %d\n", _bodyLayersPumpingParameters.pumpEveryNthLayer());
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

    int baseLayerCount( ) const {
        return _baseLayerCount;
    }

    PrintPumpingParameters& baseLayersPumpingParameters( ) {
        return _baseLayersPumpingParameters;
    }

    PrintPumpingParameters& bodyLayersPumpingParameters( ) {
        return _bodyLayersPumpingParameters;
    }

    bool baseLayersPumpingEnabled() {
        return _baseLayersPumpingEnabled;
    }

    bool bodyLayersPumpingEnabled() {
        return _bodyLayersPumpingEnabled;
    }

    //
    // Mutators
    //

    void setProfileName( QString const& newName ) {
        emit profileNameChanged( newName );
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
    int                    _baseLayerCount;
    bool                   _baseLayersPumpingEnabled;
    PrintPumpingParameters _baseLayersPumpingParameters;
    bool                   _bodyLayersPumpingEnabled;
    PrintPumpingParameters _bodyLayersPumpingParameters;

signals:
    ;

    void profileNameChanged( QString const& newName );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif // !__PRINTPROFILE_H__
