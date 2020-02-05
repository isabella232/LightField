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
