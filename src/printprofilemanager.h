#ifndef __PRINTPROFILEMANAGER_H__
#define __PRINTPROFILEMANAGER_H__

#include <QtCore>
#include "printprofile.h"

using PrintProfileCollection = QVector<QSharedPointer<PrintProfile>>;

class PrintProfileManager: public QObject
{
    Q_OBJECT

public:

    PrintProfileManager(QObject* parent = nullptr);

    virtual ~PrintProfileManager() override
    {
        /*empty*/
    }

    PrintProfileCollection* profiles()
    {
        return _profiles;
    }

    QSharedPointer<PrintProfile> activeProfile()
    {
        return _activeProfile;
    }

    bool addProfile(QSharedPointer<PrintProfile> newProfile);
    bool removeProfile(QString const& name);
    bool setActiveProfile(QString const& profileName);
    bool importProfiles(QString const& fileName);
    bool exportProfiles(QString const& fileName);
    void reload();

private:
    PrintProfileCollection* _profiles;
    QSharedPointer<PrintProfile> _activeProfile { };
    QSharedPointer<PrintProfile> _findProfile( QString const& name );
    static QSharedPointer<PrintProfile> _findProfileCustom ( QString const& name, PrintProfileCollection* profiles );

signals:
    void activeProfileChanged(QSharedPointer<PrintProfile> newProfile );
    void reloadProfiles( PrintProfileCollection* profiles );
};

#endif //!__PRINTPROFILEMANAGER_H__
