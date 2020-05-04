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

  	//MERGE TODO
    //PrintProfile( PrintProfile const& profile ): QObject( profile.parent( ) ) {
    //   _name                 = profile._name;
    //
    //    _baseLayerCount       = profile._baseLayerCount;
    //    _baseLayerParameters = profile._baseLayerParameters;
    //
    //    _bodyLayerParameters = profile._bodyLayerParameters;
    //}
  	
    PrintProfile* copy() {
        PrintProfile* printProfile = new PrintProfile();

        printProfile->_name = this->_name;
        printProfile->_baseLayerCount = this->_baseLayerCount;


        printProfile->_baseLayerParameters = this->_baseLayerParameters;
        printProfile->_bodyLayerParameters = this->_bodyLayerParameters;

        return printProfile;
    }

    void debugPrint()
    {
        debug("#############################################\n");
        debug("PrintProfile %s: \n", _name.toUtf8( ).data( ));
        debug("#############################################\n");
        debug("     objectAddress:                  %d\n", this);
        debug("     baseLayerCount                  %d\n", _baseLayerCount);
        //debug("     baseLayersParametersEnabled        %d\n", _baseLayersParametersEnabled);
        debug("     baseLayersPumpingParameters:    \n");
        debug("     objectAddress:                  %d\n", &_baseLayerParameters);
        debug("         powerLevel:                 %d\n", _baseLayerParameters.powerLevel());
        debug("         pumpUpTime:                 %d\n", _baseLayerParameters.pumpUpTime());
        debug("         pumpUpPause:                %d\n", _baseLayerParameters.pumpUpPause());
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
        debug("     bodyLayersPumpingParameters:    \n");
        debug("     objectAddress:                  %d\n", &_bodyLayerParameters);
        debug("         powerLevel:                 %d\n", _bodyLayerParameters.powerLevel());
        debug("         pumpUpTime:                 %d\n", _bodyLayerParameters.pumpUpTime());
        debug("         pumpUpPause:                %d\n", _bodyLayerParameters.pumpUpPause());
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
    bool            _defaultProfile;    

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
