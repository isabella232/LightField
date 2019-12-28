#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlpc_common.h"
#include "dlpc347x_dual.h"
#include "cypress_i2c.h"
#include "CyUSBCommon.h"

#define FLASH_WRITE_BLOCK_SIZE 1024
#define FLASH_READ_BLOCK_SIZE   256

#define MAX_WRITE_CMD_PAYLOAD  (FLASH_WRITE_BLOCK_SIZE + 8)
#define MAX_READ_CMD_PAYLOAD   (FLASH_READ_BLOCK_SIZE  + 8)

static uint8_t s_WriteBuffer[MAX_WRITE_CMD_PAYLOAD];
static uint8_t s_ReadBuffer[MAX_READ_CMD_PAYLOAD];

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

/**
 * Implement the I2C write transaction here. The sample code here sends
 * data to the controller via the Cypress USB-Serial adapter.
 */
uint32_t WriteI2C( uint16_t WriteDataLength, uint8_t* WriteData, DLPC_COMMON_CommandProtocolData_s* ProtocolData ) {
    return CYPRESS_I2C_WriteI2C( WriteDataLength, WriteData ) ? true : false;
}

/**
 * Implement the I2C write/read transaction here. The sample code here
 * receives data from the controller via the Cypress USB-Serial adapter.
 */
uint32_t WriteReadI2C( uint16_t WriteDataLength, uint8_t* WriteData, uint16_t ReadDataLength, uint8_t* ReadData, DLPC_COMMON_CommandProtocolData_s* ProtocolData ) {
    if ( !CYPRESS_I2C_WriteI2C( WriteDataLength, WriteData ) ) {
        return false;
    }

    if ( !CYPRESS_I2C_ReadI2C( ReadDataLength, ReadData ) ) {
        return false;
    }

    return true;
}

int main( int argc, char** argv ) {
    unsigned long powerLevel;
    uint32_t err;
    int ret = 0;
    bool setPowerLevel = false;

    if ( ( argc > 1 ) ) {
        if ( !StringToUnsignedLong( argv[1], 10, &powerLevel ) ) {
            fprintf( stderr, "set-projector-power: can't interpret '%s' as an integer\n", argv[1] );
            return 1;
        }
        setPowerLevel = true;
    }

    CY_RETURN_STATUS cyret;
    cyret = CyLibraryInit( );
    if ( CY_SUCCESS != cyret ) {
        fprintf( stderr, "set-projector-power: can't initialize Cypress I2C library: %d\n", cyret );
        return 1;
    }

    DLPC_COMMON_InitCommandLibrary( s_WriteBuffer, sizeof( s_WriteBuffer ), s_ReadBuffer, sizeof( s_ReadBuffer ), WriteI2C, WriteReadI2C );
    if ( !CYPRESS_I2C_ConnectToCyI2C( ) ) {
        fprintf( stderr, "set-projector-power: can't connect to Cypress I2C\n" );
        return 1;
    }
    if ( !CYPRESS_I2C_RequestI2CBusAccess( ) ) {
        fprintf( stderr, "set-projector-power: can't get access to I2C bus\n" );
        return 1;
    }

    if ( setPowerLevel ) {
        err = DLPC347X_DUAL_WriteRgbLedEnable( false, false, ( 0 == powerLevel ) );
        if ( err ) {
            fprintf( stderr, "set-projector-power: set LED enables failed: error code %d\n", err );
            ret = 1;
            goto done;
        }

        err = DLPC347X_DUAL_WriteRgbLedCurrent( 0, 0, powerLevel );
        if ( err ) {
            fprintf( stderr, "set-projector-power: set LED current failed: error code %d\n", err );
            ret = 1;
            goto done;
        }
    }

    bool re, ge, be;
    err = DLPC347X_DUAL_ReadRgbLedEnable( &re, &ge, &be );
    if ( err ) {
        fprintf( stderr, "set-projector-power: get LED enables failed: error code %d\n", err );
    } else {
        printf( "LED is %sabled\n", be ? "en" : "dis" );
    }

    uint16_t rc, gc, bc;
    err = DLPC347X_DUAL_ReadRgbLedCurrent( &rc, &gc, &bc );
    if ( err ) {
        fprintf( stderr, "set-projector-power: get LED current failed: error code %d\n", err );
    } else {
        printf( "Power level is %u\n", bc );
    }

done:
    CYPRESS_I2C_RelinquishI2CBusAccess( );
    return ret;
}
