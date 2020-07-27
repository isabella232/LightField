#include "simpleprinttest.h"

QString AbstractTest::testName = "simpletest";

SimplePrintTest::SimplePrintTest()
{

}


void SimplePrintTest::start() {
    debug("Hello test!");
}
