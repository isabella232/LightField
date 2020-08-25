#if defined _DEBUG

#ifndef PRINTPROFILETEST_H
#define PRINTPROFILETEST_H

#include <QtWidgets>
#include <QtTest/QtTest>
#include "abstracttest.h"

class PrintProfileTest: public AbstractTest
{
public:
    static QString testNameString;
    QString testName() override {
        return testNameString;
    }

    PrintProfileTest() = default;

private slots:
    void start(QString modelName);
};

#endif // PRINTPROFILETEST_H
#endif //_DEBUG
