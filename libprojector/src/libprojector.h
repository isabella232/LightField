#ifndef PROJECTORCONTROLLER
#define PROJECTORCONTROLLER

#include <stdexcept>

class ProjectorControllerException : std::exception
{
};

class ProjectorController
{
public:
    ProjectorController() = default;
    virtual ~ProjectorController() {};

    /**
     * Opens projector port, and tests it. 
     * If fails setups perror.
     * 
     * @returns true if success, false otherwise
     */
    virtual bool openPort() = 0;

    /**
     * closes projector port
     */
    virtual bool closePort() = 0;

    /**
     * Sets default boot state for power level and brightness (0)
     * 
     * @returns true if success, false otherwise
     */
    virtual bool firstTimeConfiguration() = 0;

    /**
     * Sets power level of projector
     * 
     * @returns true if success, false otherwise
     */
    virtual bool setPowerLevel(unsigned long powerLevel) = 0;

    /**
     * Sets duration of last command
     * 
     * @returns true if success, false otherwise
     */
    virtual bool setDuration(int duration) = 0;

    /**
     * @returns power level of projector
     * @throw #ProjectorControllerException if fails
     */
    virtual unsigned long getPowerLevel() = 0;

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
    virtual unsigned int getLEDBrightness() = 0;

    /**
     * @returns LED temperature (Celsius degree)
     * @throw #ProjectorControllerException if fails
     */
    virtual unsigned int getLEDTemperature() = 0;

    static ProjectorController* getInstance();
};

#endif //PROJECTORCONTROLLER