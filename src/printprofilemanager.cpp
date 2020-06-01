#include <QtCore>
#include "pch.h"

#include "profilesjsonparser.h"
#include "printprofilemanager.h"

PrintProfileManager::PrintProfileManager(QObject* parent): QObject(parent)
{
}

bool PrintProfileManager::addProfile(QSharedPointer<PrintProfile> newProfile)
{
    _profiles.insert(newProfile->profileName(), newProfile);
    ProfilesJsonParser::saveProfiles(_profiles);

    if (newProfile->isActive())
        emit activeProfileChanged(newProfile);

    emit reloadProfiles(_profiles);
    return true;
}

void PrintProfileManager::removeProfile(const QString& name)
{
    auto profile = _profiles.find(name);

    if (profile == _profiles.end())
        throw std::runtime_error("Profile not found");

    if ((*profile)->isDefault())
        throw std::runtime_error("Cannot remove default profile");

    if ((*profile)->isActive())
        throw std::runtime_error("Cannot remove active profile");

    _profiles.remove(name);
    ProfilesJsonParser::saveProfiles(_profiles);
    emit reloadProfiles(_profiles);
}

void PrintProfileManager::setActiveProfile(const QString& profileName)
{
    auto profile = _profiles.find(profileName);

    if (profile == _profiles.end())
        throw std::runtime_error("Profile not found");

    _activeProfile->setActive(false);
    (*profile)->setActive(true);
    _activeProfile = *profile;

    ProfilesJsonParser::saveProfiles(_profiles);
    emit reloadProfiles(_profiles);
}

void PrintProfileManager::reload()
{
    _profiles = ProfilesJsonParser::loadProfiles();
    QSharedPointer<PrintProfile> activeProfile = nullptr;
    QSharedPointer<PrintProfile> defaultProfile = nullptr;

    foreach (QSharedPointer<PrintProfile> profile, _profiles.values())
    {
        if (!activeProfile.isNull()) {
            /* We already have one active profile, so deactivate this one */
            profile->setActive(false);
        }

        if (profile->isActive())
            activeProfile = profile;

        if (profile->isDefault())
            defaultProfile = profile;
    }

    emit reloadProfiles(_profiles);

    if (!activeProfile.isNull()) {
        _activeProfile = activeProfile;
        emit activeProfileChanged(activeProfile);
    } else {
        _activeProfile = defaultProfile;
        defaultProfile->setActive(true );
        emit activeProfileChanged(defaultProfile);
    }
}

bool PrintProfileManager::importProfiles(const QString& mountPoint)
{
    QString sourcePath { QString("%1/print-profiles.json").arg(mountPoint) };
    QFile sourceFile(sourcePath);

    debug("+ PrintProfileManager::importProfiles\n");
    debug("  + mountPoint: %s\n", mountPoint.toUtf8().data());
    debug("  + sourcePath: %s\n", sourcePath.toUtf8().data());

    QFile::remove(PrintProfilesPath);
    return sourceFile.copy(PrintProfilesPath);
}

bool PrintProfileManager::exportProfiles(const QString& mountPoint)
{
    QString destPath { QString("%1/print-profiles.json").arg(mountPoint) };
    QFile sourceFile(PrintProfilesPath);

    debug("+ PrintProfileManager::exportProfiles\n");
    debug("  + mountPoint: %s\n", mountPoint.toUtf8().data());
    debug("  + destPath: %s\n", destPath.toUtf8().data());

    QFile::remove(destPath);
    return sourceFile.copy(destPath);
}

void PrintProfileManager::saveProfiles()
{
    ProfilesJsonParser::saveProfiles(_profiles);
}
