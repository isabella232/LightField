#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
#include <V:\usr\include\libftdi1\ftdi.h>
#else
#include <libftdi1/ftdi.h>
#endif

char const* FtdiChipTypeStrings[] {
    "TYPE_AM",
    "TYPE_BM",
    "TYPE_2232C",
    "TYPE_R",
    "TYPE_2232H",
    "TYPE_4232H",
    "TYPE_232H",
    "TYPE_230X",
};

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

bool SendCommand( ftdi_context* ftdi, uint8_t const command ) {
    int rc;

    rc = ftdi_write_data( ftdi, &command, 1 );
    if ( 1 != rc ) {
        fprintf( stderr, "set-projector-power: SendCommand/1: ftdi_write_data failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        return false;
    }
    return true;
}

bool SendCommand( ftdi_context* ftdi, uint8_t const command, uint8_t const data ) {
    int rc;

    rc = ftdi_write_data( ftdi, &command, 1 );
    if ( 1 != rc ) {
        fprintf( stderr, "set-projector-power: SendCommand/2: ftdi_write_data of command byte failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        return false;
    }

    rc = ftdi_write_data( ftdi, &data, 1 );
    if ( 1 != rc ) {
        fprintf( stderr, "set-projector-power: SendCommand/2: ftdi_write_data of data byte failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        return false;
    }

    return true;
}

bool SendCommand( ftdi_context* ftdi, uint8_t const command, uint16_t const data ) {
    int rc;

    rc = ftdi_write_data( ftdi, &command, 1 );
    if ( 1 != rc ) {
        fprintf( stderr, "set-projector-power: SendCommand/3: ftdi_write_data of command byte failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        return false;
    }

    rc = ftdi_write_data( ftdi, reinterpret_cast<uint8_t const*>( &data ), 2 );
    if ( 2 != rc ) {
        fprintf( stderr, "set-projector-power: SendCommand/3: ftdi_write_data of data word failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        return false;
    }

    return true;
}

bool FtdiSetClockDivisor( ftdi_context* ftdi, uint16_t const divisor ) {
    return SendCommand( ftdi, 0x86, divisor );
}

bool FtdiSetClockSpeed( ftdi_context* ftdi, uint32_t const speed ) {
    return FtdiSetClockDivisor( ftdi, 60'000'000 / speed );
}

bool SynchronizeMPSSE( ftdi_context* ftdi ) {
    uint8_t const BadCommandValue = 0xAA;

    fprintf( stderr, "+ SynchronizeMPSSE: sending 'bad command'\n" );
    if ( !SendCommand( ftdi, BadCommandValue ) ) {
        fprintf( stderr, "set-projector-power: SynchronizeMPSSE: SendCommand(BadCommandValue) failed\n" );
        return false;
    }

    uint8_t buf;
    int rc;
    int retries = 0;

try_again:
    // try to read until we get the byte that means 'bad command'
    do {
        usleep( 100 );
        rc = ftdi_read_data( ftdi, &buf, 1 );
        fprintf( stderr, "+ SynchronizeMPSSE: rc=%d\n", rc );
        if ( 0 == rc ) {
            retries++;
        } else if ( 1 != rc ) {
            fprintf( stderr, "set-projector-power: SynchronizeMPSSE: ftdi_read_data of command byte failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
            return false;
        } else {
            fprintf( stderr, "+ SynchronizeMPSSE: got byte 0x%02X\n", buf );
        }
    } while ( ( BadCommandValue != buf ) && ( retries < 1000 ) );

    rc = ftdi_read_data( ftdi, &buf, 1 );
    if ( 1 != rc ) {
        fprintf( stderr, "set-projector-power: SynchronizeMPSSE: ftdi_read_data of data byte failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        return false;
    }

    if ( BadCommandValue != buf ) {
        retries++;
        if ( retries == 10 ) {
            fprintf( stderr, "set-projector-power: SynchronizeMPSSE: out of retries]\n" );
            return false;
        } else {
            goto try_again;
        }
    }
    return true;
}

void Read_LedOutputControlMethod( ftdi_context* ftdi ) {

}

void Read_RgbLedEnable( ftdi_context* ftdi ) {

}

void Read_RgbLedCurrent( ftdi_context* ftdi ) {

}

void Read_RgbLedMaxCurrent( ftdi_context* ftdi ) {

}

void Read_MeasuredLedParameters( ftdi_context* ftdi ) {

}

int main( int argc, char** argv ) {
    unsigned long powerLevel    {   };
    int           ret           { 1 };
    bool          setPowerLevel {   };
    int           rc            {   };

    if ( ( argc > 1 ) ) {
        if ( !StringToUnsignedLong( argv[1], 10, &powerLevel ) ) {
            fprintf( stderr, "set-projector-power: can't interpret '%s' as an integer\n", argv[1] );
            return 1;
        }
        setPowerLevel = true;
    }

    auto ftdi { ftdi_new( ) };
    if ( !ftdi ) {
        fprintf( stderr, "set-projector-power: ftdi_new failed\n" );
        return 1;
    }

    ftdi->usb_read_timeout   = 500;
    ftdi->usb_write_timeout  = 500;
    ftdi->module_detach_mode = AUTO_DETACH_SIO_MODULE;

    rc = ftdi_usb_open( ftdi, 0x0403, 0x6001 );
    if ( rc < 0 ) {
        fprintf( stderr, "set-projector-power: unable to open FTDI USB device: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        goto bail1;
    }

    if ( ( ftdi->type >= TYPE_AM ) && ( ftdi->type <= TYPE_230X ) ) {
        fprintf( stderr, "+ chip type: %s [%d]\n", FtdiChipTypeStrings[ftdi->type], ftdi->type );
    } else {
        fprintf( stderr, "+ unknown chip type: %d\n", ftdi->type );
    }

    rc = ftdi_usb_reset( ftdi );
    if ( rc < 0 ) {
        fprintf( stderr, "set-projector-power: ftdi_usb_reset failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        goto bail2;
    }

    rc = ftdi_usb_purge_buffers( ftdi );
    if ( rc < 0 ) {
        fprintf( stderr, "set-projector-power: ftdi_usb_purge_buffers failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        goto bail2;
    }

    rc = ftdi_set_bitmode( ftdi, 0xFF, BITMODE_RESET );
    if ( rc < 0 ) {
        fprintf( stderr, "set-projector-power: ftdi_set_bitmode/1 failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        goto bail2;
    }

    rc = ftdi_set_bitmode( ftdi, 0xFF, BITMODE_MPSSE );
    if ( rc < 0 ) {
        fprintf( stderr, "set-projector-power: ftdi_set_bitmode/2 failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
        goto bail2;
    }

    if ( !SynchronizeMPSSE( ftdi ) ) {
        fprintf( stderr, "set-projector-power: SynchronizeMPSSE failed\n" );
        goto bail2;
    }

    if ( !FtdiSetClockSpeed( ftdi, 100'000 ) ) {
        fprintf( stderr, "set-projector-power: FtdiSetClockSpeed failed\n" );
        goto bail2;
    }

    //Read_LedOutputControlMethod( ftdi );
    //Read_RgbLedEnable( ftdi );
    //Read_RgbLedCurrent( ftdi );
    //Read_RgbLedMaxCurrent( ftdi );
    //Read_MeasuredLedParameters( ftdi );

    if ( setPowerLevel ) {
        //err = DLPC347X_DUAL_WriteRgbLedEnable( false, false, ( 0 == powerLevel ) );
        //if ( err ) {
        //    fprintf( stderr, "set-projector-power: set LED enables failed: error code %d\n", err );
        //    ret = 1;
        //    goto out1;
        //}
        //
        //err = DLPC347X_DUAL_WriteRgbLedCurrent( 0, 0, powerLevel );
        //if ( err ) {
        //    fprintf( stderr, "set-projector-power: set LED current failed: error code %d\n", err );
        //    ret = 1;
        //    goto out1;
        //}
    }

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

    ret = 0;

bail2:
    rc = ftdi_usb_close( ftdi );
    if ( rc < 0 ) {
        fprintf( stderr, "set-projector-power: ftdi_usb_close failed: %s [%d]\n", ftdi_get_error_string( ftdi ), rc );
    }

bail1:
    ftdi_free( ftdi );
    ftdi = nullptr;

    return ret;
}
