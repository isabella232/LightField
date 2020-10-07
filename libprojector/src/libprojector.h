#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include <stdexcept>

class ProjectorControllerException : std::exception
{
};

class ProjectorController
{
public:
    ProjectorController() = default;
    ~ProjectorController()
    {
        closePort();
    };

    /**
     * Opens projector port, and tests it. 
     * If fails setups perror.
     * 
     * @returns true if success, false otherwise
     */
    bool openPort();

    /**
     * closes projector port
     */
    bool closePort();

    /**
     * Sets default boot state for power level and brightness (0)
     * 
     * @returns true if success, false otherwise
     */
    bool firstTimeConfiguration();

    /**
     * Sets power level of projector
     * 
     * @returns true if success, false otherwise
     */
    bool setPowerLevel(unsigned long powerLevel);

    /**
     * Sets duration of last command
     * 
     * @returns true if success, false otherwise
     */
    bool setDuration(int duration);

    /**
     * @returns power level of projector
     * @throw #ProjectorControllerException if fails
     */
    unsigned long getPowerLevel();

    /**
     * auxiliary function to parse literal to unsigned long
     * 
     * @returns parsed unsinged long
     */
    static bool stringToUnsignedLong(char const *ptr, int const radix, unsigned long *result);

    /**
     * @returns LED Brightness level
     * @throw #ProjectorControllerException if fails
     */
    unsigned int getLEDBrightness();

    /**
     * @returns LED temperature (Celsius degree)
     * @throw #ProjectorControllerException if fails
     */
    unsigned int getLEDTemperature();

protected:
    bool writeCommandToProjector(char const *format, ...)
    {
        char buf[4096]{};
        va_list ap;

        va_start(ap, format);
        vsnprintf(buf, sizeof(buf) - 1, format, ap);
        va_end(ap);

        strcat(buf, "\r\n");
        auto len = strlen(buf);
        auto rc = write(fd, buf, len);

        if (rc < 0)
        {
            perror("setprojectorpower: write");
            return false;
        }
        else if (static_cast<size_t>(rc) < len)
        {
            fprintf(stderr, "+ WriteCommandToProjector: short write: %zu expected, %ld written\n", len, rc);
            return false;
        }

        return true;
    }

    bool readResponseFromProjector(char *buffer, size_t const bufferLength);

    template <typename... Args>
    bool sendCommand(char *responseBuffer, char const *format, Args... args)
    {
        for (int i = 0; i < 3; i++)
        {
            if (!writeCommandToProjector(format, args...))
            {
                fprintf(stderr, "sendCommand: WriteCommandToProjector failed\n");
                continue;
            }

            if (!readResponseFromProjector(responseBuffer, 4095))
            {
                fprintf(stderr, "sendCommand: readResponseFromProjector failed\n");
                continue;
            }

            return true;
        }

        return false;
    }

    bool isGoodResponse(char const *result);

    bool checkChecksum(unsigned const value, unsigned const checksum);

private:
    termios term{};
    int fd;
    bool monitor{};
    char buf[4096]{};
};