#ifndef PROJECTORMANAGER_H
#define PROJECTORMANAGER_H

#include "../libprojector/src/libprojector.h"

class ProjectorManager {
public:

    ProjectorManager(ProjectorController* pc) {
        this->pc = pc;
    }

    void turnOnProjector(unsigned int powerLevel) {
        if(portOpen()) {
            pc->setPowerLevel(powerLevel);
        }
    }

    void turnOnProjector(unsigned int powerLevel, int duration) {
        if(portOpen()) {
            pc->setPowerLevel(powerLevel);
            pc->setDuration(duration);
        }
    }

    void turnOffProjector() {
        if(portOpen()) {
            pc->setPowerLevel(0);
        }
    }

    bool isProjectorOn() {
        return pc->getPowerLevel() > 0;
    }
private:
    bool portOpen() {
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
