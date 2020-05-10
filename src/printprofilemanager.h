#ifndef __PRINTPROFILEMANAGER_H__
#define __PRINTPROFILEMANAGER_H__

#include <QtCore>
#include "printprofile.h"

using PrintProfileCollection = QVector<QSharedPointer<PrintProfile>>;

class PrintProfileManager: public QObject {

    Q_OBJECT;

public:

    PrintProfileManager( QObject* parent = nullptr ): QObject( parent ) {
        /*empty*/
    }

    virtual ~PrintProfileManager( ) override {
        /*empty*/
    }

    PrintProfileCollection* profiles( ) {
        return _profiles;
    }

    QSharedPointer<PrintProfile> activeProfile( ) {
        return _activeProfile;
    }

    bool addProfile(QSharedPointer<PrintProfile> newProfile);
    bool removeProfile( QString const& name );

    bool setActiveProfile( QString const& profileName );

    bool importProfiles( QString const& fileName );
    bool exportProfiles( QString const& fileName );

protected:

private:

    PrintProfileCollection* _profiles { new PrintProfileCollection };
    QSharedPointer<PrintProfile> _activeProfile { };
    QSharedPointer<PrintProfile> _findProfile( QString const& name );

signals:
    void activeProfileChanged(QSharedPointer<PrintProfile> newProfile );

};

#endif //!__PRINTPROFILEMANAGER_H__
