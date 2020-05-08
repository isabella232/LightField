#ifndef __PRINTPROFILEMANAGER_H__
#define __PRINTPROFILEMANAGER_H__

#include <QtCore>
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

    PrintProfileCollection const* profiles( ) {
        return _profiles;
    }

    PrintProfile const* activeProfile( ) {
        return _activeProfile;
    }

    bool addProfile( PrintProfile* newProfile );
    bool removeProfile( QString const& name );

    bool setActiveProfile( QString const& profileName );

    bool importProfiles( QString const& fileName );
    bool exportProfiles( QString const& fileName );

protected:

private:

    PrintProfileCollection* _profiles      { new PrintProfileCollection };
    PrintProfile*           _activeProfile { };

    PrintProfile* _findProfile( QString const& name );

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
