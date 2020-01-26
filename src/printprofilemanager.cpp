#include "pch.h"

#include "printprofilemanager.h"

PrintProfile* PrintProfileManager::_findProfile( QString const& profileName ) {
    auto iter = std::find_if( _profiles->begin( ), _profiles->end( ), [&profileName] ( PrintProfile* p ) { return profileName == p->profileName( ); } );
    return ( iter != _profiles->end( ) ) ? *iter : nullptr;
}

bool PrintProfileManager::addProfile( PrintProfile* newProfile ) {
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

bool PrintProfileManager::importProfiles( QString const& /*fileName*/ ) {
    return false;
}

bool PrintProfileManager::exportProfiles( QString const& /*fileName*/ ) {
    return false;
}
