#ifndef TESTSEXECUTOR_H
#define TESTSEXECUTOR_H

#include "abstracttest.h"
#include "simpleprinttest.h"

class TestExecutor: public QObject {
    Q_OBJECT

public:
    QMap<QString, AbstractTest*> testList { { SimplePrintTest::testName, new SimplePrintTest()} };

    void startTests(QStringList testNames) {
        debug(("TestExecutor::startTests requested tests: " + testNames.join(", ") + "\n").toUtf8().data());

        QThread* thread = QThread::create([this, testNames]{
            debug("TestExecutor::startTests thread started\n");
            for(const auto& name: testNames) {
                if(!this->testList.contains(name)) {
                    debug("There is no such test %1", name);
                    continue;
                }

                debug(QString("TestExecutor::startTests test %1 found\n").arg(name).toUtf8().data());

                AbstractTest* test = testList[name];

                QDateTime startTime;
                uint startTime_t = startTime.toTime_t();

                QObject::connect(test, &AbstractTest::successed, this, [test, startTime_t]() {
                    QDateTime endTime;
                    uint timeOverall = endTime.toTime_t() - startTime_t;

                    debug(QString(test->testName % " passed. Overall time %1 ms\n").arg(timeOverall).toUtf8().data());
                });

                QObject::connect(test, &AbstractTest::failed, this, [test, startTime_t]() {
                    QDateTime endTime;
                    uint timeOverall = endTime.toTime_t() - startTime_t;

                    debug(QString(test->testName % " failed. Overall time %1 ms\n").arg(timeOverall).toUtf8().data());
                });

                test->start();
                debug(QString("Launching test: %1\n").arg(name).toUtf8().data());
            }
        });

        thread->start();
    }
};

#endif // TESTSEXECUTOR_H
