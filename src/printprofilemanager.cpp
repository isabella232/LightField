#include <QtCore>
#include "pch.h"

#include "profilesjsonparser.h"
#include "printprofilemanager.h"

PrintProfileManager::PrintProfileManager(QObject* parent): QObject(parent)
{
}

QSharedPointer<PrintProfile> PrintProfileManager::_findProfile( QString const& profileName ) {
    auto iter = std::find_if( _profiles->begin( ), _profiles->end( ), [&profileName] ( auto p ) { return profileName == p->profileName( ); } );
    return ( iter != _profiles->end( ) ) ? *iter : nullptr;
}

QSharedPointer<PrintProfile> PrintProfileManager::_findProfileCustom ( QString const& profileName, PrintProfileCollection* profiles ) {
    auto iter = std::find_if( profiles->begin( ), profiles->end( ), [&profileName] ( auto p ) { return profileName == p->profileName( ); } );
    return ( iter != profiles->end( ) ) ? *iter : nullptr;
}

bool PrintProfileManager::addProfile(QSharedPointer<PrintProfile> newProfile)
{
    auto oldProfiles = _profiles;
    auto _profiles = ProfilesJsonParser::loadProfiles();

    newProfile->setActive( true );
    _profiles->append( newProfile );

    ProfilesJsonParser::saveProfiles( _profiles );

    emit reloadProfiles ( _profiles );
    emit activeProfileChanged( newProfile );

    delete oldProfiles;

    return true;
}

bool PrintProfileManager::removeProfile(const QString& name)
{
    auto profiles = ProfilesJsonParser::loadProfiles();
    auto profile = _findProfileCustom( name, profiles );

    if( profile->isDefault() || profile->isActive() )
        return false;

    if ( profile ) {
        _profiles->removeOne( profile );
        ProfilesJsonParser::saveProfiles( profiles );

        return true;
    } else {
        debug( "PrintProfileManager::removeProfile: can't remove profile named '%s': not in collection\n", name.toUtf8( ).data( ) );
        return false;
    }
}

bool PrintProfileManager::setActiveProfile(const QString& profileName)
{
    auto newProfile = _findProfile( profileName );
    if ( newProfile ) {
        auto oldProfiles = _profiles;
        _profiles = ProfilesJsonParser::loadProfiles();
        newProfile = _findProfile( profileName );
        newProfile->setActive( true );

        ProfilesJsonParser::saveProfiles( _profiles );
        delete oldProfiles;

        emit reloadProfiles ( _profiles );
        emit activeProfileChanged( newProfile );
        _activeProfile = newProfile;
        return true;
    } else {
        debug( "PrintProfileManager::setActiveProfile: couldn't find profile named '%s' in collection\n", profileName.toUtf8( ).data( ) );
        return false;
    }
}

void PrintProfileManager::reload() {
    _profiles = ProfilesJsonParser::loadProfiles();
    QSharedPointer<PrintProfile> activeProfile = nullptr;
    QSharedPointer<PrintProfile> defaultProfile;

    foreach ( QSharedPointer<PrintProfile> profile, _profiles ) {
        if( profile->isActive() )
            activeProfile = profile;

        if( profile->isDefault() )
            defaultProfile = profile;
    }

    emit reloadProfiles ( _profiles );

    if( activeProfile != nullptr ) {
        _activeProfile = activeProfile;
        emit activeProfileChanged( activeProfile );
    } else {
        _activeProfile = defaultProfile;
        defaultProfile->setActive( true );
        emit activeProfileChanged( defaultProfile );
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

bool PrintProfileManager::exportProfiles(const QString& mountPoint )
{
    QString destPath { QString("%1/print-profiles.json").arg(mountPoint) };
    QFile sourceFile(PrintProfilesPath);

    debug("+ PrintProfileManager::exportProfiles\n");
    debug("  + mountPoint: %s\n", mountPoint.toUtf8().data());
    debug("  + destPath: %s\n", destPath.toUtf8().data());

    QFile::remove(destPath);
    return sourceFile.copy(destPath);
}
