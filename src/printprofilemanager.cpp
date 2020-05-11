#include "pch.h"

#include "profilesjsonparser.h"
#include "printprofilemanager.h"

QSharedPointer<PrintProfile> PrintProfileManager::_findProfile( QString const& profileName ) {
    auto iter = std::find_if( _profiles->begin( ), _profiles->end( ), [&profileName] ( auto p ) { return profileName == p->profileName( ); } );
    return ( iter != _profiles->end( ) ) ? *iter : nullptr;
}


bool PrintProfileManager::addProfile(QSharedPointer<PrintProfile> newProfile ) {
    auto profile = _findProfile( newProfile->profileName( ) );
    if ( !profile ) {
        _profiles->append( newProfile );
        return true;
    } else {
        debug( "PrintProfileManager::addProfile: can't add profile %p: already have a profile named '%s'\n", newProfile, newProfile->profileName( ).toUtf8( ).data( ) );
        return false;
    }
}

bool PrintProfileManager::removeProfile( QString const& name ) {
    auto profile = _findProfile( name );
    if ( profile ) {
        _profiles->removeOne( profile );
        return true;
    } else {
        debug( "PrintProfileManager::removeProfile: can't remove profile named '%s': not in collection\n", name.toUtf8( ).data( ) );
        return false;
    }
}

bool PrintProfileManager::setActiveProfile( QString const& profileName ) {
    auto newProfile = _findProfile( profileName );
    if ( newProfile ) {
        emit activeProfileChanged( newProfile );
        _activeProfile = newProfile;
        return true;
    } else {
        debug( "PrintProfileManager::setActiveProfile: couldn't find profile named '%s' in collection\n", profileName.toUtf8( ).data( ) );
        return false;
    }
}

bool PrintProfileManager::importProfiles(const QString& mountPoint)
{
    QString sourcePath { QString("%1/print-profiles.json").arg(mountPoint) };
    QFile sourceFile(sourcePath);

    debug("+ PrintProfileManager::importProfiles\n");
    debug("  + mountPoint: %s\n", mountPoint.toUtf8().data());
    debug("  + sourcePath: %s\n", sourcePath.toUtf8().data());

    if (!QFile::remove(PrintProfilesPath))
        return false;

    return sourceFile.copy(PrintProfilesPath);
}

bool PrintProfileManager::exportProfiles(const QString& mountPoint )
{
    QString destPath { QString("%1/print-profiles.json").arg(mountPoint) };
    QFile sourceFile(PrintProfilesPath);

    debug("+ PrintProfileManager::importProfiles\n");
    debug("  + mountPoint: %s\n", mountPoint.toUtf8().data());
    debug("  + destPath: %s\n", destPath.toUtf8().data());

    if (!QFile::remove(destPath))
        return false;

    return sourceFile.copy(destPath);
}
