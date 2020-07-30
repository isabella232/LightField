#include "simpleprinttest.h"
#include "../window.h"
#include "../filetab.h"
#include "../printtab.h"

QString AbstractTest::testName = "simpletest";


void SimplePrintTest::start() {

    debug("  +SimplePrintTest::start!\n");

    waitForMainWindow();

    fileSelectModelOnList();

    fileClickSelectButton();

    prepareSliceButtonClick();

    if(!g_settings.pretendPrinterIsPrepared) {
        //@todo
    } else {
        //switch to print tab
        PrintTab* printTab = findWidget<PrintTab*>("print");
        printTab->show();
        QTest::qWait(1000);
    }

    //continue clicked
    QPushButton* printContinue = findWidget<QPushButton*>("printPrint");
    mouseClick(printContinue);


    //start print click
    QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
    mouseClick(statusStartPrint);

    debug("+end test\n");
}
