#if defined _DEBUG

#include "tilingtest.h"
#include "../window.h"
#include "../filetab.h"
#include "../printtab.h"

QString TilingTest::testNameString = "tilingtest";

void TilingTest::start(QString modelName = nullptr) {

    debug("=================================Tiling Test=================================!\n");

    waitForMainWindow();

    fileSelectModelOnList();

    fileClickSelectButton();

    tilingSwitchToTilingWindow();

    tilingSetupTilingButton();

    int expectedMaxWCount = tilingSetSpacing();

    tilingSetCount(expectedMaxWCount);

    tilingSetExposureTime();

    tilingCreateTiles();

    if(!g_settings.pretendPrinterIsPrepared) {
        prepareClickPrepare();
        prepareClickContinue();
    } else {
        //switch to print tab
        PrintTab* printTab = findWidget<PrintTab*>("print");
        printTab->show();
        QTest::qWait(1000);
    }

    printContinueButtonClick();

    statusStartPrint();

    emit successed();
    debug("===============================Tiling Test finished============================!\n");
}

#endif //_DEBUG
