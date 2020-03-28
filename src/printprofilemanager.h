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

    PrintProfileCollection const* profiles( ) const {
        return _profiles;
    }

    PrintProfile* activeProfile( ) {
        return _activeProfile;
    }

    PrintProfile* getProfile( QString const& name );

    bool addProfile( PrintProfile* newProfile );
    bool removeProfile( QString const& name );

    bool setActiveProfile( QString const& name );

    bool importProfiles( QString const& fileName );
    bool exportProfiles( QString const& fileName );

protected:

private:

    PrintProfileCollection* _profiles      { new PrintProfileCollection };
    PrintProfile*           _activeProfile { };

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
