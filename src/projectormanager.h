#ifndef PROJECTORMANAGER_H
#define PROJECTORMANAGER_H

#include "../libprojector/src/libprojector.h"
#include "debug.h"

class ProjectorManager {
public:

    ProjectorManager(ProjectorController* pc) {
        this->pc = pc;
    }

    void turnOnProjector(unsigned int powerLevel) {
        debug("+ ProjectorManager::turnOnProjector %d\n", powerLevel);

        if(portOpen()) {
            pc->setPowerLevel(powerLevel);
        }
    }

    void turnOnProjector(unsigned int powerLevel, int duration) {
        debug("+ ProjectorManager::turnOnProjector %d, duration:  %d ms\n", powerLevel, duration);
        if(portOpen()) {
            pc->setPowerLevel(powerLevel);
            pc->setDuration(duration);
        }
    }

    void turnOffProjector() {
        debug("+ ProjectorManager::turnOffProjector\n");

        if(portOpen()) {
            pc->setPowerLevel(0);
        }
    }

    bool isProjectorOn() {
        debug("+ ProjectorManager::isProjectorOn\n");

        return pc->getPowerLevel() > 0;
    }
private:
    bool portOpen() {
        debug("+ ProjectorManager::portOpen\n");

        if(!isPortOpen) {
            isPortOpen = pc->openPort();
        }

        return isPortOpen;
    }
    bool isPortOpen { false };

    ProjectorController* pc;
};

extern ProjectorManager* projectorManager;

#endif // PROJECTORMANAGER_H
