#include "simpleprinttest.h"
#include "../window.h"
#include "../filetab.h"
#include "../printtab.h"

QString AbstractTest::testName = "simpletest";


void SimplePrintTest::start() {

    debug("=================================Simple Print Test=================================!\n");

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

    printContinueButtonClick();

    statusStartPrint();

    emit successed();
    debug("===============================Simple Print Test finished============================!\n");
}
