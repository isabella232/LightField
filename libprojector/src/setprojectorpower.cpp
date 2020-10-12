#include "libprojector.h"

#include <getopt.h>
#include <signal.h>
#include <unistd.h>

namespace Option
{
    int const Help = 'h';
    int const FirstTime = 'f';
    int const Monitor = 'm';
} // namespace Option

char const shortOptions[]{":h?fm"};

option const longOptions[]{
    {"help", no_argument, nullptr, Option::Help},
    {"first-time", no_argument, nullptr, Option::FirstTime},
    {"monitor", no_argument, nullptr, Option::Monitor},
};

enum class Mode
{
    QueryPowerLevel,
    SetPowerLevel,
    FirstTimeConfiguration,
};

[[noreturn]] void printUsageAndExit(bool success = false)
{
    fprintf(stderr,
            "Usage: setprojectorpower --first-time\n"
            "   or: setprojectorpower [--monitor] [<value> [<duration>]]\n"
            "Where:\n"
            "  -h, -?, --help     display this help and exit\n"
            "  -f, --first-time   perform first-time projector configuration\n"
            "  -m, --monitor      monitor projector brightness and temperature\n"
            "  <value>            brightness, range 0..1023\n"
            "  <duration>         milliseconds, range 1..65535\n");
    exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
}

void monitorPowerAndTemperature(ProjectorController* pc)
{
    unsigned checksum;
    unsigned brightness;
    unsigned temperature;
    siginfo_t sigInfo;
    sigset_t sigset;
    int sig;
    timespec const timeout{1, 0};
    char buf[4096]{};

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGHUP);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGQUIT);
    sigaddset(&sigset, SIGTERM);

    while (true)
    {
        try
        {
            //
            // Query LED brightness
            //
            brightness = pc->getLEDBrightness();

            //
            // Query LED temperature
            //
            temperature = pc->getLEDTemperature();

            printf("\rbrightness: %4u, temperature: %4u °C", brightness, temperature);
        }
        catch (ProjectorControllerException &e)
        {
            sig = sigtimedwait(&sigset, &sigInfo, &timeout);
            if (-1 == sig)
            {
                if ((EAGAIN == errno) || (EINTR == errno))
                {
                    continue;
                }
                else
                {
                    fputc('\n', stdout);
                    perror("setprojectorpower: sigtimedwait");
                }
            }
            else if (SIGINT == sig)
            {
                fputc('\n', stdout);
                exit(0);
            }
            else
            {
                fprintf(stderr, "\n+ sigtimedwait: got unknown signal %d?\n", sig);
            }
        }
    }
}

int main(int argc, char **argv)
{
    Mode mode{Mode::QueryPowerLevel};
    unsigned long duration{0};
    unsigned long powerLevel{};
    bool monitor{};
    ProjectorController* pc  = ProjectorController::getInstance();

    setvbuf(stdout, nullptr, _IONBF, 0);

    int opt;
    do
    {
        opt = getopt_long(argc, argv, shortOptions, longOptions, nullptr);
        if (-1 == opt)
        {
            break;
        }

        if (Option::FirstTime == opt)
        {
            if (monitor)
            {
                fprintf(stderr, "setprojectorpower: options --first-time and --monitor may not be combined\n");
                printUsageAndExit();
            }
            mode = Mode::FirstTimeConfiguration;
        }
        else if (Option::Monitor == opt)
        {
            if (Mode::FirstTimeConfiguration == mode)
            {
                fprintf(stderr, "setprojectorpower: options --first-time and --monitor may not be combined\n");
                printUsageAndExit();
            }
            monitor = true;
        }
        else if (Option::Help == opt)
        {
            printUsageAndExit(true);
        }
        else
        {
            printUsageAndExit();
        }
    } while (opt != -1);

    if (Mode::FirstTimeConfiguration != mode)
    {
        if (optind < argc)
        {
            if (ProjectorController::stringToUnsignedLong(argv[optind], 10, &powerLevel))
            {
                if (powerLevel > 1023)
                {
                    fprintf(stderr, "setprojectorpower: brightness value %lu out of range\n", powerLevel);
                    printUsageAndExit();
                }
                mode = Mode::SetPowerLevel;
            }
            ++optind;
        }
        if (optind < argc)
        {
            if (ProjectorController::stringToUnsignedLong(argv[optind], 10, &duration))
            {
                if ((0 == duration) || (duration > 65535))
                {
                    fprintf(stderr, "setprojectorpower: duration value %lu out of range\n", duration);
                    printUsageAndExit();
                }
            }
            ++optind;
        }
    }

    if (!pc->openPort())
    {
        return -1;
    }

    switch (mode)
    {
    case Mode::FirstTimeConfiguration:
        pc->firstTimeConfiguration();

        break;

    case Mode::SetPowerLevel:
        pc->setPowerLevel(powerLevel);

        if (duration > 0)
        {
            pc->setDuration(duration);
        }

    case Mode::QueryPowerLevel:
        try
        {
            unsigned long pl = pc->getPowerLevel();

            printf("LED brightness: %lu\n", pl);
        }
        catch (ProjectorControllerException &e)
        {
        }

        break;
    }

    if (monitor)
    {
        monitorPowerAndTemperature(pc);
    }
}