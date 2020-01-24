#ifndef __PRINTPROFILEMANAGER_H__
#define __PRINTPROFILEMANAGER_H__

#include "printprofile.h"

using PrintProfileCollection = QVector<PrintProfile*>;

class PrintProfileManager: public QObject {

    Q_OBJECT;

public:

    PrintProfileManager( QObject* parent = nullptr ): QObject( parent ) {
        /*empty*/
    }

    virtual ~PrintProfileManager( ) override {
        /*empty*/
    }

    bool importProfiles( QString const& fileName );
    bool exportProfiles( QString const& fileName );

    PrintProfile const* activeProfile( ) {
        return _activeProfile;
    }

    void setActiveProfile( PrintProfile* newProfile ) {
        emit activeProfileChanged( newProfile );
        _activeProfile = newProfile;
    }

    PrintProfileCollection const* profiles( ) {
        return _profiles;
    }

protected:

private:

    PrintProfile*           _activeProfile { };
    PrintProfileCollection* _profiles      { new PrintProfileCollection };

signals:
    ;

    void activeProfileChanged( PrintProfile const* newProfile );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif //!__PRINTPROFILEMANAGER_H__
