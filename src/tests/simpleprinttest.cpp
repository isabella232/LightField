#include "simpleprinttest.h"
#include "../window.h"
#include "../filetab.h"

QString AbstractTest::testName = "simpletest";

SimplePrintTest::SimplePrintTest()
{

}


void SimplePrintTest::start() {
    debug("+SimplePrintTest::Hello test!\n");
    Window* window = App::mainWindow();
    QWidget* selectButton = (QWidget*)findWidgetByNameRoot("fileSelect");

    mouseClick(selectButton);

    emit AbstractTest::successed();
}
