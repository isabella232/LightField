#include "pch.h"

#include "printprofilemanager.h"

PrintProfile* PrintProfileManager::getProfile( QString const& profileName ) {
    auto iter = std::find_if( _profiles->begin( ), _profiles->end( ), [&profileName] ( PrintProfile* p ) { return profileName == p->profileName( ); } );
    return ( iter != _profiles->end( ) ) ? *iter : nullptr;
}

bool PrintProfileManager::addProfile( PrintProfile* newProfile ) {
    if ( auto profile = getProfile( newProfile->profileName( ) ); !profile ) {
        _profiles->append( newProfile );
        return true;
    } else {
        debug( "PrintProfileManager::addProfile: can't add profile: already have one named '%s'\n", newProfile->profileName( ).toUtf8( ).data( ) );
        return false;
    }
}

bool PrintProfileManager::removeProfile( QString const& name ) {
    if ( auto profile = getProfile( name ); profile ) {
        _profiles->removeOne( profile );
        return true;
    } else {
        debug( "PrintProfileManager::removeProfile: can't remove profile named '%s': not in collection\n", name.toUtf8( ).data( ) );
        return false;
    }
}

bool PrintProfileManager::setActiveProfile( QString const& name ) {
    if ( auto profile = getProfile( name ); profile ) {
        emit activeProfileChanged( profile );
        _activeProfile = profile;
        return true;
    } else {
        debug( "PrintProfileManager::setActiveProfile: can't find profile named '%s' in collection\n", name.toUtf8( ).data( ) );
        return false;
    }
}

bool PrintProfileManager::importProfiles( QString const& /*fileName*/ ) {
    return false;
}

bool PrintProfileManager::exportProfiles( QString const& /*fileName*/ ) {
    return false;
}
