#include <QtCore>
#include "pch.h"

#include "profilesjsonparser.h"
#include "printprofilemanager.h"

PrintProfileManager::PrintProfileManager(QObject* parent): QObject(parent)
{
}

bool PrintProfileManager::addProfile(QSharedPointer<PrintProfile> newProfile)
{
    auto profiles = ProfilesJsonParser::loadProfiles();
    QString activeProfileName = _activeProfile->profileName();

    loadProfile(activeProfileName);

    _profiles.insert(newProfile->profileName(), newProfile);
    profiles.insert(newProfile->profileName(), newProfile);

    newProfile->setDefault(false);

    foreach (QSharedPointer<PrintProfile> profile, _profiles.values()) {
        profile->setActive(false);
    }

    newProfile->setActive(true);
    _activeProfile = newProfile;

    ProfilesJsonParser::saveProfiles(profiles);
    emit activeProfileChanged(newProfile);

    return true;
}

void PrintProfileManager::renameProfile(const QString& oldName, const QString& newName) {
    auto profiles = ProfilesJsonParser::loadProfiles();
    auto profile = profiles.find(oldName);

    if (profile == _profiles.end())
        throw std::runtime_error("Profile not found");

    if((*profile)->isActive()) {
        throw std::runtime_error("Cannot rename active profile");
    }

    if((*profile)->isDefault()) {
        throw std::runtime_error("Cannot rename default profile");
    }

    foreach (QSharedPointer<PrintProfile> iter, _profiles.values()) {
        if (iter->profileName() == newName) {
            throw std::runtime_error("A profile with the same name already exists.");
        }
    }

    (*profile)->setProfileName(newName);

    ProfilesJsonParser::saveProfiles(profiles);
    auto insideProfile = _profiles.find(oldName);
    (*insideProfile)->setProfileName(newName);

    emit reloadProfiles(_profiles);
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

    auto profiles = ProfilesJsonParser::loadProfiles();
    profiles.remove(name);
    _profiles.remove(name);

    ProfilesJsonParser::saveProfiles(profiles);
    emit reloadProfiles(_profiles);
}

void PrintProfileManager::setActiveProfile(const QString& profileName)
{
    auto profile = _profiles.find(profileName);
    auto profiles = ProfilesJsonParser::loadProfiles();

    if (profile == _profiles.end())
        throw std::runtime_error("Profile not found");

    _activeProfile->setActive(false);
    (*profile)->setActive(true);

    (*profiles.find(profileName))->setActive(true);
    (*profiles.find(_activeProfile->profileName()))->setActive(false);

    _activeProfile = *profile;

    ProfilesJsonParser::saveProfiles(profiles);

    emit activeProfileChanged(*profile);
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
        defaultProfile->setActive(true);
        emit activeProfileChanged(defaultProfile);
    }
}

void PrintProfileManager::loadProfile(const QString& profileName)
{
    auto profiles = ProfilesJsonParser::loadProfiles();
    auto profile = _profiles.find(profileName);

    if (profile == _profiles.end())
        throw std::runtime_error("Profile not found");

    profile->swap(*profiles.find(profileName));
}

void PrintProfileManager::saveProfile(const QString& profileName)
{
    auto profiles = ProfilesJsonParser::loadProfiles();
    auto profile = _profiles.find(profileName);

    if (profile == _profiles.end())
        throw std::runtime_error("Profile not found");

    QSharedPointer<PrintProfile> profileCopy { new PrintProfile(*_activeProfile) };

    profileCopy->setProfileName(profileName);
    profileCopy->setDefault(false);

    profiles.insert((*profile)->profileName(), profileCopy);
    ProfilesJsonParser::saveProfiles(profiles);
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
