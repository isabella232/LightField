#ifndef SIMPLEPRINTTEST_H
#define SIMPLEPRINTTEST_H

#include <QtWidgets>
#include <QtTest/QtTest>
#include "abstracttest.h"

class SimplePrintTest: public AbstractTest
{
public:
    SimplePrintTest() = default;

private slots:
    void start();
};

#endif // SIMPLEPRINTTEST_H
