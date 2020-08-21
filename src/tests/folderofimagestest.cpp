#if defined _DEBUG
#include "folderofimagestest.h"
#include "../window.h"
#include "../filetab.h"
#include "../printtab.h"

QString FolderOfImagesTest::testNameString = "folderofimagestest";

void FolderOfImagesTest::start(QString modelName = nullptr) {

    TDEBUG("=================================Folder Of Images Test=================================!\n");
    TDEBUG("MODEL: %s", modelName.toUtf8().data());
    waitForMainWindow();

    fileSelectModelOnList(modelName, true);

    fileClickSelectButton();

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
    TDEBUG("===============================Folder Of Images Test finished============================!\n");
}

#endif //_DEBUG
