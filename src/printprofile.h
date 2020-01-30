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

protected:

private:

    QString                _name;
    int                    _baseLayerCount;
    PrintPumpingParameters _baseLayersPumpingParameters;
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
