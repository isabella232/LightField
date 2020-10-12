#ifndef PROJECTORDLP4710IMPL
#define PROJECTORDLP4710IMPL

#include "libprojector.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>

class ProjectorDlp4710Impl: public ProjectorController {
public:
    unsigned int getLEDBrightness() override;
    bool openPort() override;
    bool closePort() override;
    bool firstTimeConfiguration() override;
    bool setPowerLevel(unsigned long powerLevel) override;
    bool setDuration(int duration) override;
    unsigned long getPowerLevel() override;
    unsigned int getLEDTemperature() override;
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

#endif //PROJECTORDLP4710IMPL