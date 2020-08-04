#if defined _DEBUG

#ifndef SIMPLEPRINTTEST_H
#define SIMPLEPRINTTEST_H

#include <QtWidgets>
#include <QtTest/QtTest>
#include "abstracttest.h"

class SimplePrintTest: public AbstractTest
{
public:
    static QString testNameString;
    QString testName() override {
        return testNameString;
    }

    SimplePrintTest() = default;

private slots:
    void start();
};

#endif // SIMPLEPRINTTEST_H
#endif //_DEBUG
