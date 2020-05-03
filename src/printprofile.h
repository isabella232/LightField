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


        printProfile->_baseLayerParameters = this->_baseLayersParameters;
        printProfile->_bodyLayerParameters = this->_bodyLayersParameters;

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
        debug("     objectAddress:                  %d\n", &_baseLayersParameters);
        debug("         powerLevel:                 %d\n", _baseLayersParameters.powerLevel());
        debug("         pumpUpTime:                 %d\n", _baseLayersParameters.pumpUpTime());
        debug("         pumpUpPause:                %d\n", _baseLayersParameters.pumpUpPause());
        debug("         pumpDownPause:              %d\n", _baseLayersParameters.pumpDownPause());
        debug("         layerThickness:             %d\n", _baseLayersParameters.layerThickness());
        debug("         pumpUpDistance:             %d\n", _baseLayersParameters.pumpUpDistance());
        debug("         noPumpUpVelocity:           %d\n", _baseLayersParameters.noPumpUpVelocity());
        debug("         layerExposureTime:          %d\n", _baseLayersParameters.layerExposureTime());
        debug("         pumpEveryNthLayer:          %d\n", _baseLayersParameters.pumpEveryNthLayer());
        /*debug("         pumpDownTime_Effective:     %d\n", _baseLayersPumpingParameters.pumpDownTime_Effective());
        debug("         pumpUpVelocity_Effective:   %d\n", _baseLayersPumpingParameters.pumpUpVelocity_Effective());
        debug("         pumpDownDistance_Effective: %d\n", _baseLayersPumpingParameters.pumpDownDistance_Effective());
        debug("         pumpDownVelocity_Effective: %d\n", _baseLayersPumpingParameters.pumpDownVelocity_Effective());*/
        //debug("     bodyLayersParametersEnabled        %d\n", _bodyLayersParametersEnabled);
        debug("     bodyLayersPumpingParameters:    \n");
        debug("     objectAddress:                  %d\n", &_bodyLayersParameters);
        debug("         powerLevel:                 %d\n", _bodyLayersParameters.powerLevel());
        debug("         pumpUpTime:                 %d\n", _bodyLayersParameters.pumpUpTime());
        debug("         pumpUpPause:                %d\n", _bodyLayersParameters.pumpUpPause());
        debug("         pumpDownPause:              %d\n", _bodyLayersParameters.pumpDownPause());
        debug("         layerThickness:             %d\n", _bodyLayersParameters.layerThickness());
        debug("         pumpUpDistance:             %d\n", _bodyLayersParameters.pumpUpDistance());
        debug("         noPumpUpVelocity:           %d\n", _bodyLayersParameters.noPumpUpVelocity());
        debug("         layerExposureTime:          %d\n", _bodyLayersParameters.layerExposureTime());
        debug("         pumpEveryNthLayer:          %d\n", _bodyLayersParameters.pumpEveryNthLayer());
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
