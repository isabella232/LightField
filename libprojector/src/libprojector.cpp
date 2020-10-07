#include "libprojector.h"

bool ProjectorController::stringToUnsignedLong(char const *ptr, int const radix, unsigned long *result)
{
    char *endptr = NULL;

    errno = 0;
    unsigned long ret = strtoul(ptr, &endptr, radix);
    if (endptr != (ptr + strlen(ptr)))
    {
        return false;
    }
    if (errno == ERANGE && ret == ULONG_MAX)
    {
        return false;
    }
    if (errno != 0 && ret == 0UL)
    {
        return false;
    }

    *result = ret;
    return true;
}

bool ProjectorController::readResponseFromProjector(char *buffer, size_t const bufferLength)
{
    fd_set read_fds, write_fds, except_fds;
    struct timeval timeout;
    size_t index = 0;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    FD_SET(fd, &read_fds);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    memset(buffer, '\0', bufferLength);

    while (1)
    {
        if (select(fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
        {
            auto rc = read(fd, &buffer[index], 1);

            if (rc < 0)
            {
                perror("setprojectorpower: read");
                return false;
            }
            else if (rc == 0)
            {
                fprintf(stderr, "+ ReadResponseFromProjector: EOF??\n");
                return false;
            }

            if ((index > 0) && (buffer[index - 1] == '\r') && (buffer[index] == '\n'))
            {
                buffer[index - 1] = '\0';
                return true;
            }

            ++index;

            if (index == bufferLength)
                return false;
        }
        else
        {
            return false;
        }
    }
}

bool ProjectorController::isGoodResponse(char const *result)
{
    return (0 != strcmp("ERROR", result));
}

bool ProjectorController::checkChecksum(unsigned const value, unsigned const checksum)
{
    unsigned int tempChecksum = 0;
    unsigned int tempValue = value;

    while (tempValue > 0)
    {
        tempChecksum += tempValue % 10;
        tempValue /= 10;
    }

    return (checksum == tempChecksum);
}

unsigned int ProjectorController::getLEDBrightness()
{
    unsigned int brightness = 0;
    unsigned int checksum = 0;

    if (!sendCommand(buf, "WT+GLGT"))
    {
        fprintf(stderr, "\nsetprojectorpower: command WT+GLGT: failed to send command");
        return 0;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "\nsetprojectorpower: command WT+GLGT: got negative response '%s'\n", buf);
        return 0;
    }

    errno = 0;
    if (2 != sscanf(buf, "OK:%u:%u", &brightness, &checksum))
    {
        auto err = errno;
        if (err == 0)
        {
            fprintf(stderr, "\nsetprojectorpower: command WT+GLGT: couldn't parse response '%s'\n", buf);
        }
        else
        {
            fprintf(stderr, "\nsetprojectorpower: command WT+GLGT: couldn't parse response '%s': %s [%d]\n", buf, strerror(err), err);
        }
        return 0;
    }

    if (!checkChecksum(brightness, checksum))
    {
        fprintf(stderr, "\nsetprojectorpower: command WT+GLGT: got bad checksum\n");

        return 0;
    }

    return brightness;
}

bool ProjectorController::openPort()
{
    fd = open("/dev/lumen-projector", O_RDWR);

    if (-1 == fd)
    {
        perror("setprojectorpower: open");

        return false;
    }
    else if (-1 == tcgetattr(fd, &term))
    {
        perror("setprojectorpower: tcgetattr");

        closePort();
        return false;
    }
    else if (-1 == cfsetispeed(&term, B115200))
    {
        perror("setprojectorpower: cfsetispeed");

        closePort();
        return false;
    }
    else if (-1 == cfsetospeed(&term, B115200))
    {
        perror("setprojectorpower: cfsetospeed");

        closePort();
        return false;
    }

    cfmakeraw(&term);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 5;

    if (-1 == tcsetattr(fd, TCSANOW, &term))
    {
        perror("setprojectorpower: tcsetattr");
        closePort();

        return false;
    }

    if (!sendCommand(buf, "WT+PWRE=%d", 1))
    {
        fprintf(stderr, "setprojectorpower: command WT+PWRE: failed to send command");
        closePort();

        return false;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+PWRE: got negative response ('%s')\n", buf);
        closePort();

        return false;
    }

    return (bool)fd;
}

bool ProjectorController::closePort()
{
    return close(fd);
}

bool ProjectorController::firstTimeConfiguration()
{
    printf("Setting default boot state\n");
    if (!sendCommand(buf, "WT+SPWR=%d", 1))
    {
        fprintf(stderr, "setprojectorpower: command WT+SPWR: failed to send command");
        return false;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+SPWR: got negative response ('%s')\n", buf);
        return false;
    }

    printf("Setting default LED state\n");
    if (!sendCommand(buf, "WT+SLED=%d", 0))
    {
        fprintf(stderr, "setprojectorpower: command WT+SLED: failed to send command");

        return false;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+SLED: got negative response ('%s')\n", buf);

        return false;
    }

    printf("Setting default brightness\n");
    if (!sendCommand(buf, "WT+SBTN=%d", 0))
    {
        fprintf(stderr, "setprojectorpower: command WT+SBTN: failed to send command");

        return false;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+SBTN: got negative response ('%s')\n", buf);

        return false;
    }

    return true;
}

bool ProjectorController::setPowerLevel(unsigned long powerLevel)
{
    if (!sendCommand(buf, "WT+LEDE=%d", (0 == powerLevel) ? 0 : 1))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDE: failed to send command");

        return false;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDE: got negative response ('%s')\n", buf);

        return false;
    }

    if (!sendCommand(buf, "WT+LEDS=%lu", powerLevel))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDS: failed to send command");

        return false;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDS: got negative response ('%s')\n", buf);

        return false;
    }

    return true;
}

bool ProjectorController::setDuration(int duration)
{
    if (!sendCommand(buf, "WT+LEDT=%lu", duration))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDT: failed to send command");
        return false;
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDT: got negative response ('%s')\n", buf);
        return false;
    }

    return true;
}

unsigned long ProjectorController::getPowerLevel()
{
    unsigned long powerLevel{};

    if (!sendCommand(buf, "WT+LEDR"))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDR: failed to send command");

        throw ProjectorControllerException();
    }

    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDR: got negative response ('%s')\n", buf);

        throw ProjectorControllerException();
    }

    if (!stringToUnsignedLong(buf, 10, &powerLevel))
    {
        fprintf(stderr, "setprojectorpower: command WT+LEDR: couldn't convert '%s' to a number\n", buf);

        throw ProjectorControllerException();
    }

    return powerLevel;
}

unsigned int ProjectorController::getLEDTemperature()
{
    unsigned int temperature;
    unsigned int checksum;

    if (!sendCommand(buf, "WT+GTMP"))
    {
        fprintf(stderr, "\nsetprojectorpower: command WT+GTMP: failed to send command");

        throw ProjectorControllerException();
    }
    if (!isGoodResponse(buf))
    {
        fprintf(stderr, "\nsetprojectorpower: command WT+GTMP: got negative response '%s'\n", buf);

        throw ProjectorControllerException();
    }

    errno = 0;
    if (2 != sscanf(buf, "OK:%u:%u", &temperature, &checksum))
    {
        auto err = errno;
        if (err == 0)
        {
            fprintf(stderr, "\nsetprojectorpower: command WT+GTMP: couldn't parse response '%s'\n", buf);
        }
        else
        {
            fprintf(stderr, "\nsetprojectorpower: command WT+GTMP: couldn't parse response '%s': %s [%d]\n", buf, strerror(err), err);
        }

        throw ProjectorControllerException();
    }
    if (!checkChecksum(temperature, checksum))
    {
        fprintf(stderr, "\nsetprojectorpower: command WT+GTMP: got bad checksum\n");

        throw ProjectorControllerException();
    }

    return temperature;
}