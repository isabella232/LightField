#ifndef PROJECTORMANAGER_H
#define PROJECTORMANAGER_H

#include "../libprojector/src/libprojector.h"
#include "debug.h"
#include "qthread.h"

class ProjectorManager: public QObject {
Q_OBJECT

public:

    ProjectorManager(ProjectorController* pc) {
        this->pc = pc;
    }

    void turnOnProjector(unsigned int powerLevel) {
        debug("+ ProjectorManager::turnOnProjector %d\n", powerLevel);

        QThread *thread = QThread::create([this, powerLevel] {
            if(portOpen()) {
                if(pc->setPowerLevel(powerLevel)) {
                    emit turnOnProjectorDone();
                }
            }

            emit turnOnProjectorFailed();
        });
        thread->start();
    }

    void turnOnProjector(unsigned int powerLevel, int duration) {
        debug("+ ProjectorManager::turnOnProjector %d, duration:  %d ms\n", powerLevel, duration);

        QThread *thread = QThread::create([this, powerLevel, duration] {
            if(portOpen()) {
                if(pc->setPowerLevel(powerLevel) && pc->setDuration(duration)) {
                    emit turnOffProjectorDone();
                }
            }

            emit turnOnProjectorFailed();
        });
        thread->start();
    }

    void turnOffProjector() {
        debug("+ ProjectorManager::turnOffProjector\n");

        QThread *thread = QThread::create([this] {
            if(portOpen() && pc->setPowerLevel(0)) {
                emit turnOffProjectorDone();
            }

            emit turnOnProjectorFailed();
        });
        thread->start();
    }

    bool isProjectorOn() {
        debug("+ ProjectorManager::isProjectorOn\n");

        return pc->getPowerLevel() > 0;
    }

signals:
    void turnOnProjectorDone();
    void turnOnProjectorFailed();
    void turnOffProjectorDone();
    void turnOffProjectorFailed();

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
