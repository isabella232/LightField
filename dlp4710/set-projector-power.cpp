#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

namespace {

    enum class Mode {
        QueryPowerLevel,
        SetPowerLevel,
        FirstTimeConfiguration,
    };

    namespace Option {
        int const Help      = 'h';
        int const FirstTime = 'f';
        int const Monitor   = 'm';
    }

    char const ShortOptions[] { ":h?fm" };

    option const LongOptions[] {
        { "help",       no_argument, nullptr, Option::Help      },
        { "first-time", no_argument, nullptr, Option::FirstTime },
        { "monitor",    no_argument, nullptr, Option::Monitor   },
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

    template<typename... Args>
    bool WriteCommandToProjector( int const fd, char const* format, Args... args ) {
        char buf[4096] { };
        snprintf( buf, 4093, format, args... );
        strcat( buf, "\r\n" );
        auto len = strlen( buf );
        auto rc  = write( fd, buf, len );
        if ( rc < 0 ) {
            perror( "set-projector-power: write" );
            return false;
        } else if ( static_cast<size_t>( rc ) < len ) {
            fprintf( stderr, "+ WriteCommandToProjector: short write: %zu expected, %ld written\n", len, rc );
            return false;
        }
        return true;
    }

    bool ReadResponseFromProjector( int const fd, char* buffer, size_t const bufferLength ) {
        size_t index = 0;

        memset( buffer, '\0', bufferLength );
        while ( index < bufferLength ) {
            auto rc = read( fd, &buffer[index], 1 );
            if ( rc < 0 ) {
                perror( "set-projector-power: read" );
                return false;
            } else if ( rc == 0 ) {
                fprintf( stderr, "+ ReadResponseFromProjector: EOF??\n" );
                return false;
            }

            if ( ( index > 0 ) && ( buffer[index - 1] == '\r' ) && ( buffer[index] == '\n' ) ) {
                buffer[index - 1] = '\0';
                return true;
            }

            ++index;
        }
        return true;
    }

    template<typename... Args>
    bool SendCommand( int const fd, char* responseBuffer, char const* format, Args... args ) {
        if ( !WriteCommandToProjector( fd, format, args... ) ) {
            fprintf( stderr, "SendCommand: WriteCommandToProjector failed\n" );
            return false;
        }

        if ( !ReadResponseFromProjector( fd, responseBuffer, 4095 ) ) {
            fprintf( stderr, "SendCommand: ReadResponseFromProjector failed\n" );
            return false;
        }

        return true;
    }

    bool IsGoodResponse( char const* result ) {
        return ( 0 != strcmp( "ERROR", result ) );
    }

    bool CheckChecksum( unsigned const value, unsigned const checksum ) {
        unsigned tempChecksum = 0;
        unsigned tempValue    = value;

        while ( tempValue > 0 ) {
            tempChecksum += tempValue % 10;
            tempValue /= 10;
        }

        return ( checksum == tempChecksum );
    }

    [[noreturn]]
    void PrintUsageAndExit( bool success = false ) {
        fprintf( stderr,
            "Usage: set-projector-power --first-time\n"
            "   or: set-projector-power [--monitor] [<value> [<duration>]]\n"
            "Where:\n"
            "  -h, -?, --help     display this help and exit\n"
            "  -f, --first-time   perform first-time projector configuration\n"
            "  -m, --monitor      monitor projector brightness and temperature\n"
            "  <value>            brightness, range 0..1023\n"
            "  <duration>         milliseconds, range 1..65535\n"
        );
        exit( success ? EXIT_SUCCESS : EXIT_FAILURE );
    }

    void MonitorPowerAndTemperature( int const fd ) {
        unsigned       checksum;
        unsigned       brightness;
        unsigned       temperature;
        siginfo_t      sigInfo;
        sigset_t       sigset;
        int            sig;
        timespec const timeout     { 1, 0 };
        char           buf[4096]   { };

        sigemptyset( &sigset );
        sigaddset( &sigset, SIGHUP  );
        sigaddset( &sigset, SIGINT  );
        sigaddset( &sigset, SIGQUIT );
        sigaddset( &sigset, SIGTERM );

        while ( true ) {
            //
            // Query LED brightness
            //

            if ( !SendCommand( fd, buf, "WT+GLGT" ) ) {
                fprintf( stderr, "\nset-projector-power: command WT+GLGT: failed to send command" );
                return;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "\nset-projector-power: command WT+GLGT: got negative response '%s'\n", buf );
                return;
            }

            errno = 0;
            if ( 2 != sscanf( buf, "OK:%u:%u", &brightness, &checksum ) ) {
                auto err = errno;
                if ( err == 0 ) {
                    fprintf( stderr, "\nset-projector-power: command WT+GLGT: couldn't parse response '%s'\n", buf );
                } else {
                    fprintf( stderr, "\nset-projector-power: command WT+GLGT: couldn't parse response '%s': %s [%d]\n", buf, strerror( err ), err );
                }
                return;
            }
            if ( !CheckChecksum( brightness, checksum ) ) {
                fprintf( stderr, "\nset-projector-power: command WT+GLGT: got bad checksum\n" );
                goto next;
            }

            //
            // Query LED temperature
            //

            if ( !SendCommand( fd, buf, "WT+GTMP" ) ) {
                fprintf( stderr, "\nset-projector-power: command WT+GTMP: failed to send command" );
                return;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "\nset-projector-power: command WT+GTMP: got negative response '%s'\n", buf );
                return;
            }

            errno = 0;
            if ( 2 != sscanf( buf, "OK:%u:%u", &temperature, &checksum ) ) {
                auto err = errno;
                if ( err == 0 ) {
                    fprintf( stderr, "\nset-projector-power: command WT+GTMP: couldn't parse response '%s'\n", buf );
                } else {
                    fprintf( stderr, "\nset-projector-power: command WT+GTMP: couldn't parse response '%s': %s [%d]\n", buf, strerror( err ), err );
                }
                return;
            }
            if ( !CheckChecksum( temperature, checksum ) ) {
                fprintf( stderr, "\nset-projector-power: command WT+GTMP: got bad checksum\n" );
                goto next;
            }

            printf( "\rbrightness: %4u, temperature: %4u °C", brightness, temperature );

        next:
            sig = sigtimedwait( &sigset, &sigInfo, &timeout );
            if ( -1 == sig ) {
                if ( ( EAGAIN == errno ) || ( EINTR == errno ) ) {
                    continue;
                } else {
                    fputc( '\n', stdout );
                    perror( "set-projector-power: sigtimedwait" );
                }
            } else if ( SIGINT == sig ) {
                fputc( '\n', stdout );
                exit( 0 );
            } else {
                fprintf( stderr, "\n+ sigtimedwait: got unknown signal %d?\n", sig );
            }
        }
    }

}

int main( int argc, char** argv ) {
    Mode          mode       { Mode::QueryPowerLevel };
    unsigned long powerLevel { };
    unsigned long duration   { 0 };
    int           ret        { 1 };
    char          buf[4096]  { };
    int           fd         { };
    termios       term       { };
    bool          monitor    { };

    setvbuf( stdout, nullptr, _IONBF, 0 );

    int opt;
    do {
        opt = getopt_long( argc, argv, ShortOptions, LongOptions, nullptr );
        if ( -1 == opt ) {
            break;
        }

        if ( Option::FirstTime == opt ) {
            if ( monitor ) {
                fprintf( stderr, "set-projector-power: options --first-time and --monitor may not be combined\n" );
                PrintUsageAndExit( );
            }
            mode = Mode::FirstTimeConfiguration;
        } else if ( Option::Monitor == opt ) {
            if ( Mode::FirstTimeConfiguration == mode ) {
                fprintf( stderr, "set-projector-power: options --first-time and --monitor may not be combined\n" );
                PrintUsageAndExit( );
            }
            monitor = true;
        } else if ( Option::Help == opt ) {
            PrintUsageAndExit( true );
        } else {
            PrintUsageAndExit( );
        }
    } while ( opt != -1 );

    if ( Mode::FirstTimeConfiguration != mode ) {
        if ( optind < argc ) {
            if ( StringToUnsignedLong( argv[optind], 10, &powerLevel ) ) {
                if ( powerLevel > 1023 ) {
                    fprintf( stderr, "set-projector-power: brightness value %lu out of range\n", powerLevel );
                    PrintUsageAndExit( );
                }
                mode = Mode::SetPowerLevel;
            }
            ++optind;
        }
        if ( optind < argc ) {
            if ( StringToUnsignedLong( argv[optind], 10, &duration ) ) {
                if ( ( 0 == duration ) || ( duration > 65535 ) ) {
                    fprintf( stderr, "set-projector-power: duration value %lu out of range\n", duration );
                    PrintUsageAndExit( );
                }
            }
            ++optind;
        }
    }

    fd = open( "/dev/lumen-projector", O_RDWR );
    if ( -1 == fd ) {
        perror( "set-projector-power: open" );
        goto bail1;
    }

    if ( -1 == tcgetattr( fd, &term ) ) {
        perror( "set-projector-power: tcgetattr" );
        goto bail2;
    }

    if ( -1 == cfsetispeed( &term, B115200 ) ) {
        perror( "set-projector-power: cfsetispeed" );
        goto bail2;
    }
    if ( -1 == cfsetospeed( &term, B115200 ) ) {
        perror( "set-projector-power: cfsetospeed" );
        goto bail2;
    }

    cfmakeraw( &term );
    term.c_cc[VMIN]  = 1;
    term.c_cc[VTIME] = 5;

    if ( -1 == tcsetattr( fd, TCSANOW, &term ) ) {
        perror( "set-projector-power: tcsetattr" );
        goto bail2;
    }

    if ( !SendCommand( fd, buf, "WT+PWRE=%d", 1 ) ) {
        fprintf( stderr, "set-projector-power: command WT+PWRE: failed to send command" );
        goto bail2;
    }
    if ( !IsGoodResponse( buf ) ) {
        fprintf( stderr, "set-projector-power: command WT+PWRE: got negative response ('%s')\n", buf );
        goto bail2;
    }

    switch ( mode ) {
        case Mode::FirstTimeConfiguration:
            printf( "Setting default boot state\n" );
            if ( !SendCommand( fd, buf, "WT+SPWR=%d", 1 ) ) {
                fprintf( stderr, "set-projector-power: command WT+SPWR: failed to send command" );
                goto bail2;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "set-projector-power: command WT+SPWR: got negative response ('%s')\n", buf );
                goto bail2;
            }

            printf( "Setting default LED state\n" );
            if ( !SendCommand( fd, buf, "WT+SLED=%d", 1 ) ) {
                fprintf( stderr, "set-projector-power: command WT+SLED: failed to send command" );
                goto bail2;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "set-projector-power: command WT+SLED: got negative response ('%s')\n", buf );
                goto bail2;
            }

            printf( "Setting default brightness\n" );
            if ( !SendCommand( fd, buf, "WT+SBTN=%d", 0 ) ) {
                fprintf( stderr, "set-projector-power: command WT+SBTN: failed to send command" );
                goto bail2;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "set-projector-power: command WT+SBTN: got negative response ('%s')\n", buf );
                goto bail2;
            }

            printf( "Done!\n" );
            break;

        case Mode::SetPowerLevel:
            if ( !SendCommand( fd, buf, "WT+LEDE=%d", ( 0 == powerLevel ) ? 0 : 1 ) ) {
                fprintf( stderr, "set-projector-power: command WT+LEDE: failed to send command" );
                goto bail2;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "set-projector-power: command WT+LEDE: got negative response ('%s')\n", buf );
                goto bail2;
            }

            if ( !SendCommand( fd, buf, "WT+LEDS=%lu", powerLevel ) ) {
                fprintf( stderr, "set-projector-power: command WT+LEDS: failed to send command" );
                goto bail2;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "set-projector-power: command WT+LEDS: got negative response ('%s')\n", buf );
                goto bail2;
            }

            if ( duration > 0 ) {
                if ( !SendCommand( fd, buf, "WT+LEDT=%lu", duration ) ) {
                    fprintf( stderr, "set-projector-power: command WT+LEDT: failed to send command" );
                    goto bail2;
                }
                if ( !IsGoodResponse( buf ) ) {
                    fprintf( stderr, "set-projector-power: command WT+LEDT: got negative response ('%s')\n", buf );
                    goto bail2;
                }
            }
            // FALLTHROUGH

        case Mode::QueryPowerLevel:
            if ( !SendCommand( fd, buf, "WT+LEDR" ) ) {
                fprintf( stderr, "set-projector-power: command WT+LEDR: failed to send command" );
                goto bail2;
            }
            if ( !IsGoodResponse( buf ) ) {
                fprintf( stderr, "set-projector-power: command WT+LEDR: got negative response ('%s')\n", buf );
                goto bail2;
            }
            if ( !StringToUnsignedLong( buf, 10, &powerLevel ) ) {
                fprintf( stderr, "set-projector-power: command WT+LEDR: couldn't convert '%s' to a number\n", buf );
                goto bail2;
            }
            printf( "LED brightness: %lu\n", powerLevel );
            break;
    }

    if ( monitor ) {
        MonitorPowerAndTemperature( fd );
    }

    ret = 0;

bail2:
    close( fd );

bail1:
    return ret;
}
