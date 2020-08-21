#if defined _DEBUG
#ifndef EXTENDEDPRINTTEST_H
#define EXTENDEDPRINTTEST_H

#include <QtWidgets>
#include <QtTest/QtTest>
#include "abstracttest.h"

class ExtendedPrintTest: public AbstractTest
{
public:
    static QString testNameString;
    QString testName() override {
        return testNameString;
    }

    ExtendedPrintTest() = default;

private slots:
    void start(QString modelName);
};

#endif // EXTENDEDPRINTTEST_H
#endif //_DEBUG
