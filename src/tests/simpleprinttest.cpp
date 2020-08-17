#if defined _DEBUG

#include "simpleprinttest.h"
#include "../window.h"
#include "../filetab.h"
#include "../printtab.h"

QString SimplePrintTest::testNameString = "simpletest";

void SimplePrintTest::start() {

    TDEBUG("=================================Simple Print Test=================================!\n");

    waitForMainWindow();

    fileSelectModelOnList();

    fileClickSelectButton();

    prepareSliceButtonClick();

    QLabel* prepareMsg = findWidget<QLabel*>("preparePrepareMessage");

    if(!g_settings.pretendPrinterIsPrepared && prepareMsg->text() != "Preparation completed.") {
        QPushButton* prepareButton = findWidget<QPushButton*>("preparePrepare");

        if(prepareButton->text() == "Prepare...")
            prepareClickPrepare();

        prepareClickContinue();
    } else {
        //switch to print tab
        PrintTab* printTab = findWidget<PrintTab*>("print");
        printTab->show();
        QTest::qWait(1000);
    }

    setupBasicExposureTime();

    printContinueButtonClick();

    statusStartPrint();

    emit successed();
    TDEBUG("===============================Simple Print Test finished============================!\n");
}

#endif //_DEBUG
