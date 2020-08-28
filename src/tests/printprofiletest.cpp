#if defined _DEBUG

#include "printprofiletest.h"
#include "../window.h"
#include "../filetab.h"
#include "../printtab.h"

QString PrintProfileTest::testNameString = "printprofiletest";

void PrintProfileTest::start(QString modelName = nullptr) {
    (void)modelName;
    TDEBUG("=================================Print Profile Test=================================!\n");

    waitForMainWindow();

    switchToPrintProfileTab();

    createPrintProfile();


    emit successed();
    TDEBUG("==============================Print Profile Test finished===========================!\n");
}

#endif //_DEBUG
