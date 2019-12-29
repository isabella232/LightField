#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libMPSSE_i2c.h"

bool StringToUnsignedLong( char const* ptr, int const radix, unsigned long* result ) {
    char* endptr = NULL;

    errno = 0;
    unsigned long ret = strtoul( ptr, &endptr, radix );
    if ( endptr != ( ptr + strlen( ptr ) ) ) {
        return false;
    }
    if ( errno == ERANGE && ret == ULONG_MAX ) {
        return false;
    }
    if ( errno != 0 && ret == 0UL ) {
        return false;
    }
    *result = ret;
    return true;
}

int main( int argc, char** argv ) {
    unsigned long powerLevel    { };
    //uint32_t      err           { };
    int           ret           { };
    //bool          setPowerLevel { };
    ChannelConfig channelConf   { };
    FT_STATUS     status        { };
    FT_HANDLE     ftHandle      { };
    uint32        channels      { };

    if ( ( argc > 1 ) ) {
        if ( !StringToUnsignedLong( argv[1], 10, &powerLevel ) ) {
            fprintf( stderr, "set-projector-power: can't interpret '%s' as an integer\n", argv[1] );
            return 1;
        }
        //setPowerLevel = true;
    }

    //Init_libMPSSE( );

    channelConf.ClockRate    = I2C_CLOCK_FAST_MODE;
    channelConf.LatencyTimer = 255;

    status = I2C_GetNumChannels( &channels );
    if ( FT_OK != status ) {
        fprintf( stderr, "set-projector-power: I2C_GetNumChannels failed: %u\n", status );
        ret = 1;
        goto out1;
    }
    if ( 0 == channels ) {
        fprintf( stderr, "set-projector-power: no I2C channels available\n" );
        ret = 1;
        goto out1;
    }

    for ( uint32 n = 0; n < channels; ++n ) {
        FT_DEVICE_LIST_INFO_NODE devList { };

        status = I2C_GetChannelInfo( n, &devList );
        if ( FT_OK != status ) {
            fprintf( stderr, "set-projector-power: I2C_GetChannelInfo failed on channel %u: %u\n", n, status );
            ret = 1;
            goto out1;
        }

        printf(
            "Channel %d:\n"
            "  Flags:        0x%08X\n"
            "  Type:         0x%08X\n"
            "  ID:           0x%08X\n"
            "  LocId:        0x%08X\n"
            "  SerialNumber: '%.16s'\n"
            "  Description:  '%.64s'\n"
            "  ftHandle:     %p\n"
            "",
            n,
            devList.Flags,
            devList.Type,
            devList.ID,
            devList.LocId,
            devList.SerialNumber,
            devList.Description,
            devList.ftHandle
        );
    }

    status = I2C_OpenChannel( 0, &ftHandle );
    if ( FT_OK != status ) {
        fprintf( stderr, "set-projector-power: I2C_OpenChannel failed: %u\n", status );
        ret = 1;
        goto out1;
    }

    status = I2C_InitChannel( ftHandle, &channelConf );
    if ( FT_OK != status ) {
        fprintf( stderr, "set-projector-power: I2C_InitChannel failed: %u\n", status );
        ret = 1;
        goto out2;
    }












    // ===========================================
    //
    //if ( setPowerLevel ) {
    //    err = DLPC347X_DUAL_WriteRgbLedEnable( false, false, ( 0 == powerLevel ) );
    //    if ( err ) {
    //        fprintf( stderr, "set-projector-power: set LED enables failed: error code %d\n", err );
    //        ret = 1;
    //        goto out1;
    //    }
    //
    //    err = DLPC347X_DUAL_WriteRgbLedCurrent( 0, 0, powerLevel );
    //    if ( err ) {
    //        fprintf( stderr, "set-projector-power: set LED current failed: error code %d\n", err );
    //        ret = 1;
    //        goto out1;
    //    }
    //}
    //
    //bool re, ge, be;
    //err = DLPC347X_DUAL_ReadRgbLedEnable( &re, &ge, &be );
    //if ( err ) {
    //    fprintf( stderr, "set-projector-power: get LED enables failed: error code %d\n", err );
    //} else {
    //    printf( "LED is %sabled\n", be ? "en" : "dis" );
    //}
    //
    //uint16_t rc, gc, bc;
    //err = DLPC347X_DUAL_ReadRgbLedCurrent( &rc, &gc, &bc );
    //if ( err ) {
    //    fprintf( stderr, "set-projector-power: get LED current failed: error code %d\n", err );
    //} else {
    //    printf( "Power level is %u\n", bc );
    //}
    //
    // ===========================================




out2:
    status = I2C_CloseChannel( ftHandle );
    if ( FT_OK != status ) {
        fprintf( stderr, "set-projector-power: I2C_CloseChannel failed: %u\n", status );
        ret = 1;
    }

out1:
    //Cleanup_libMPSSE( );
    return ret;
}
