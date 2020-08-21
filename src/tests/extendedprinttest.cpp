#if defined _DEBUG

#include "extendedprinttest.h"
#include "../printtab.h"

QString ExtendedPrintTest::testNameString = "extendedtest";


void ExtendedPrintTest::start(QString modelName = nullptr) {

    TDEBUG("=================================Extended Print Test=================================!\n");
    waitForMainWindow();

    fileSelectModelOnList(modelName);

    fileClickSelectButton();

    setCustomThickness();

    prepareSliceButtonClick();

    QLabel* prepareMsg = findWidget<QLabel*>("preparePrepareMessage");

    TDEBUG(prepareMsg->text().toUtf8().data());
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

    setupExposureTime();

    pauseStopPrintTest();

    statusStartPrint();

    emit successed();
    TDEBUG("===============================Extended Print Test Finished============================!\n");
}
#endif
