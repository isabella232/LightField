#if defined _DEBUG

#include "extendedprinttest.h"
#include "../printtab.h"

QString ExtendedPrintTest::testNameString = "extendedtest";


void ExtendedPrintTest::start() {

    debug("=================================Simple Print Test=================================!\n");
    waitForMainWindow();

    fileSelectModelOnList();

    fileClickSelectButton();

    setCustomThickness();

    prepareSliceButtonClick();

    if(!g_settings.pretendPrinterIsPrepared) {
        prepareClickPrepare();
        prepareClickContinue();
    } else {
        //switch to print tab
        PrintTab* printTab = findWidget<PrintTab*>("print");
        printTab->show();
        QTest::qWait(1000);
    }

    setupExposureTime();

    pauseStopPrintTest();

    statusStartPrint();

    emit successed();
    debug("===============================Simple Print Test finished============================!\n");
}
#endif
