#if defined _DEBUG

#ifndef TILINGTEST_H
#define TILINGTEST_H

#include <QtWidgets>
#include <QtTest/QtTest>
#include "abstracttest.h"

class TilingTest: public AbstractTest
{
public:
    static QString testNameString;
    QString testName() override {
        return testNameString;
    }

    TilingTest() = default;

private slots:
    void start();
};

#endif // TILINGTEST_H
#endif //_DEBUG
