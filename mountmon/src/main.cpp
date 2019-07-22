#include "pch.h"

#include "commandreader.h"
#include "udisksmonitor.h"
#include "usbdevicemounter.h"

uid_t LightFieldUserId  = -1;
gid_t LightFieldGroupId = -1;

namespace {

    char const* DefaultUserName  { "lumen" };
    char const* DefaultGroupName { "lumen" };

    unsigned NegativeOne = static_cast<unsigned>( -1 );

    uid_t _GetUserIdForName( QString const& userName ) {
        errno = 0;
        passwd* result = ::getpwnam( userName.toUtf8( ).data( ) );
        if ( !result ) {
            debug( "+ _GetUserIdForName: getpwnam failed: %s [%d]\n", strerror( errno ), errno );
            return NegativeOne;
        }
        return result->pw_uid;
    }

    gid_t _GetGroupIdForName( QString const& groupName ) {
        group* result = ::getgrnam( groupName.toUtf8( ).data( ) );
        if ( !result ) {
            debug( "+ GetGroupIdForName: getgrnam_r failed?! %s [%d]\n", strerror( errno ), errno );
            return false;
        }
        return result->gr_gid;
    }

    void _GetLightFieldIds( ) {
        auto environment { QProcessEnvironment::systemEnvironment( ) };
        auto sudoUid     { environment.value( "SUDO_UID"  )          };
        auto sudoGid     { environment.value( "SUDO_GID"  )          };
        bool ok;

        LightFieldUserId = sudoUid.toUInt( &ok );
        if ( !ok ) {
            debug( "+ _GetLightFieldIds: couldn't retrieve user ID from environment variable SUDO_UID, falling back to default user name '%s'\n", DefaultUserName );
            LightFieldUserId = _GetUserIdForName( DefaultUserName );
            if ( NegativeOne == LightFieldUserId ) {
                debug( "+ _GetLightFieldIds: couldn't retrieve user ID via default user name either\n" );
            }
        }

        LightFieldGroupId = sudoGid.toUInt( &ok );
        if ( !ok ) {
            debug( "+ _GetLightFieldIds: couldn't retrieve group ID from environment variable SUDO_GID, falling back to default group name '%s'\n", DefaultGroupName );
            LightFieldGroupId = _GetGroupIdForName( DefaultGroupName );
            if ( NegativeOne == LightFieldGroupId ) {
                debug( "+ _GetLightFieldIds: couldn't retrieve group ID via default group name either\n" );
            }
        }

        debug( "+ _GetLightFieldIds: uid=%u gid=%u\n", LightFieldUserId, LightFieldGroupId );
        if ( NegativeOne == LightFieldUserId || NegativeOne == LightFieldGroupId ) {
            debug( "+ _GetLightFieldIds: FATAL: Couldn't look up user ID or group ID for LightField user!!\n" );
            ::exit( 2 );
        }
    }
}

int main( int argc, char** argv ) {
    setvbuf( stdout, nullptr, _IONBF, 0 );

    _GetLightFieldIds( );

    auto             app              { QCoreApplication { argc, argv } };
    CommandReader    commandReader    { &app                            };
    UDisksMonitor    udisksMonitor    { &app                            };
    UsbDeviceMounter usbDeviceMounter { udisksMonitor, &app             };

    (void) QObject::connect( &commandReader, &CommandReader::commandReceived, &usbDeviceMounter, &UsbDeviceMounter::commandReader_commandReceived );

    app.exec( );
}
