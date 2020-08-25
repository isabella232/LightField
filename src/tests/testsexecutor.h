#if defined _DEBUG

#ifndef TESTSEXECUTOR_H
#define TESTSEXECUTOR_H

#include "abstracttest.h"
#include "simpleprinttest.h"
#include "extendedprinttest.h"
#include "tilingtest.h"
#include "folderofimagestest.h"
#include "printprofiletest.h"

class TestExecutor: public QObject {
    Q_OBJECT

public:
    QMap<QString, AbstractTest*> testList {
        {SimplePrintTest::testNameString, new SimplePrintTest()},
        {ExtendedPrintTest::testNameString, new ExtendedPrintTest()},
        {TilingTest::testNameString, new TilingTest()},
        {FolderOfImagesTest::testNameString, new FolderOfImagesTest()},
        {PrintProfileTest::testNameString, new PrintProfileTest()}
    };

    TestExecutor() {

        int number = 0;
        std::stringstream nameStream;
        std::string name;
        char* name_c_str;

        nameStream << TestLogPath << ".log";
        name = nameStream.str();
        name_c_str = (char*)name.c_str();

        while(FILE *file = fopen(name_c_str, "r")) {
            fclose(file);
            number++;
            nameStream.str("");
            name.clear();
            nameStream << TestLogPath << "_" << number << ".log";
            name = nameStream.str();
            name_c_str = (char*)name.c_str();
        }

        TestLog = ::fopen( name_c_str, "wa" );
        ::setvbuf(TestLog, nullptr, _IONBF, 0);
    }

    void startTests(QStringList testNames, QString modelName=nullptr) {
        debug(("+TestExecutor::startTests requested tests: " + testNames.join(", ") + "\n").toUtf8().data());
        QThread* thread = QThread::create([this, testNames, modelName]{
            debug("+TestExecutor::startTests thread started\n");
            for(const auto name: testNames) {
                if(!this->testList.contains(name)) {
                    debug("There is no such test %1", name);
                    continue;
                }

                debug(QString("+TestExecutor::startTests test %1 found\n").arg(name).toUtf8().data());

                AbstractTest* test = testList[name];

                QDateTime startTime = QDateTime::currentDateTime();
                uint startTime_t = startTime.toTime_t();

                QObject::connect(test, &AbstractTest::successed, this, [test, &startTime_t]() {
                    QDateTime endTime = QDateTime::currentDateTime();
                    uint timeOverall = endTime.toTime_t() - startTime_t;

                    test->TDEBUG(QString(test->testName() % " passed. Overall time %1 s\n").arg(timeOverall).toUtf8().data());
                });

                QObject::connect(test, &AbstractTest::failed, this, [test, &startTime_t](QString msg) {
                    QDateTime endTime = QDateTime::currentDateTime();
                    uint timeOverall = endTime.toTime_t() - startTime_t;

                    test->TDEBUG(QString(test->testName() % " failed. %2. Overall time %1 s \n").arg(timeOverall).arg(msg).toUtf8().data());
                });

                try {
                    test->start(modelName);
                } catch (QException e) {
                    QTest::qWait(2000);
                }
            }
        });

        thread->start();
    }
};

#endif // TESTSEXECUTOR_H
#endif //_DEBUG
