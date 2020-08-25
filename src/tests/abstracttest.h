#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

#include <QtTest>

#include <ctime>
#include <cstdlib>
#include <cstdio>

#include <iomanip>
#include "../window.h"
#include "../filetab.h"
#include "../thicknesswindow.h"
#include "../canvas.h"
#include "../keyboard.h"
#include "../key.h"
#include "../spoiler.h"
#include "../progressdialog.h"
#include "../inputdialog.h"
#include <iostream>

extern FILE* TestLog;

class AbstractTest: public QObject {
    Q_OBJECT
public:
    AbstractTest() = default;

    virtual QString testName() = 0;

    virtual void start(QString modelName = nullptr) = 0;

    void tDebug( char const* function, char const* str ) {
        time_t timeNow = std::time(nullptr);

        std::stringstream ss;

        ss << std::put_time(std::localtime(&timeNow), "%OH:%OM:%OS | ") << "[" << function << "]: " << str << "\n";

        const std::string formattedOutput = ss.str();
        const char* cstr = formattedOutput.c_str();

        if ( TestLog ) {
            ::fputs(cstr, TestLog);
            ::fflush(TestLog);
        }

        ss.str("");
        ss << "============ TEST: " << str << "\n";

        const std::string formattedOutput2 = ss.str();
        const char* cstr2 = formattedOutput2.c_str();

        debug(cstr2);
    }

    template<typename... Args>
    inline void tDebug(char const* function, char const* fmt, Args... args ) {
        if ( char* buf; asprintf( &buf, fmt, args... ) > 0 ) {
            tDebug( function, buf );
            free( buf );
        }
    }

#define TDEBUG(...) tDebug(__PRETTY_FUNCTION__, __VA_ARGS__)

    bool ASSERT(bool arg1, bool halt=true, QString msg = "", QString file="", QString line="") {
        if(arg1 && halt)
        {
            QString alert = QString("false predicate: [%1]:%2 %3").arg(file).arg(line).arg(msg);

            emit failed(alert);
            throw QException();
        }
        return true;
    }
#define xstr0(s) str0(s)
#define str0(s) #s

#define T_ASSERT(cond, halt, msg) ((cond) ? true : ASSERT(#cond, #halt, #msg, __FILE__, __LINE__))
#define S_ASSERT(cond) ((cond) ? true : ASSERT(#cond, true, QString(str0(cond)), QString(__FILE__), QString(__LINE__)))

    void dispatchToMainThread(std::function<void()> callback)
    {
        // any thread
        QTimer* timer = new QTimer();
        timer->moveToThread(qApp->thread());
        timer->setSingleShot(true);
        QObject::connect(timer, &QTimer::timeout, [=]()
        {
            // main thread
            callback();
            timer->deleteLater();
        });
        QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
    }

    void mouseClick(QWidget* widget) {
        bool done = false;
        dispatchToMainThread([widget, &done]() {
            QTest::mouseClick(widget, Qt::MouseButton::LeftButton);
            done = true;
        });

        QTest::qWaitFor([&done]() {
            QApplication::processEvents();
            return done;
        }, 5000);

        QTest::qWait(1000);
    }

    template <typename T>
    T findWidget(QString name) {
        Window* window = App::mainWindow();

        return window->findChild<T>(name, Qt::FindChildOption::FindChildrenRecursively);
    }

    template <typename T>
    T findWidget(QWidget* parent, QString name) {
        return parent->findChild<T>(name, Qt::FindChildOption::FindChildrenRecursively);
    }

    template <typename T>
    T findWidgetInTopLevel(QString name) {
        QWidgetList widgetList = QApplication::topLevelWidgets();
        T result = nullptr;
        for(QWidget* w: widgetList) {
            if(w->objectName() == name) {
                result = (T)w;
            }
        }

        return result;
    }

    void clickOkMsgBoxTopLevel() {
        QWidgetList allToplevelWidgets = QApplication::topLevelWidgets();
        foreach (QWidget *w, allToplevelWidgets) {
            if (w->inherits("QMessageBox")) {
                QMessageBox *mb = qobject_cast<QMessageBox *>(w);
                QTest::keyClick(mb, Qt::Key_Enter);
            }
        }
    }

    QString getRandomString() const
    {
       const QString possibleCharacters("abcdefghijklmnopqrstuvwxyz");
       const int randomStringLength = 5; // assuming you want random strings of 12 characters

       QString randomString;
       for(int i=0; i<randomStringLength; ++i)
       {
           int index = qrand() % possibleCharacters.length();
           QChar nextChar = possibleCharacters.at(index);
           randomString.append(nextChar);
       }
       return randomString;
    }

    QString getRandomPrintProfiletName() {
        return QString("test profile ") % getRandomString();
    }

    //waiting until main window is ready
    void waitForMainWindow() {
        Window* window = App::mainWindow();

        QTest::qWaitFor([window]() {
            return !window->isInitialized();
        }, 5000);

        QTabWidget* tabs = findWidget<QTabWidget*>("tabWidget");
        tabs->setCurrentIndex(1);

        S_ASSERT(window->isInitialized());
    }

    //selecting model on list
    void fileSelectModelOnList(QString modelName = nullptr, bool folder = false) {
        TDEBUG("file select model on list ");
        GestureListView* availableFilesListView = findWidget<GestureListView*>("fileAvailableFiles");
        FileTab* fileTab = findWidget<FileTab*>("file");
        Canvas* canvas = findWidget<Canvas*>("fileCanvas");
        QPushButton* deleteButton = findWidget<QPushButton*>("fileDelete");
        QWidget* selectButton = findWidget<QPushButton*>("fileSelect");
        QRadioButton* fileViewSolid = findWidget<QRadioButton*>("fileViewSolid");
        QRadioButton* fileViewWireframe = findWidget<QRadioButton*>("fileViewWireframe");
        QLabel* fileDimensions = findWidget<QLabel*>("fileDimensions");

        QFileSystemModel* model = fileTab->libraryFsModel();
        QString fullPath = QString("/var/lib/lightfield/model-library/%1").arg(modelName != nullptr ? modelName : "Menger_sponge_sample.stl");
        QModelIndex index = model->index(fullPath);
        availableFilesListView->setCurrentIndex(model->index(fullPath));
        availableFilesListView->clicked(index);

        QTest::qWait(1000);

        S_ASSERT(!canvas->objectName().isEmpty());
        S_ASSERT(deleteButton->isEnabled());
        S_ASSERT(selectButton->isEnabled());

        if (!folder) {
            S_ASSERT(fileViewSolid->isEnabled());
            S_ASSERT(fileViewWireframe->isEnabled());

            QRegExp rx("\\d+\\.\\d+ mm × \\d+\\.\\d+ mm × \\d+\\.\\d+ mm, \\d+\\.\\d+ µL");
            S_ASSERT(rx.indexIn(fileDimensions->text()) != -1);
            TDEBUG("file select model on list - PASSED ");
        }
    }

    void fileClickSelectButton() {
        TDEBUG("select button file tab ");
        Window* window = App::mainWindow();
        QTabWidget* tabs = findWidget<QTabWidget*>("tabWidget");

        QWidget* selectButton = findWidget<QPushButton*>("fileSelect");
        QPushButton* resliceButton = findWidget<QPushButton*>("prepareSlice");

        QTest::qWaitFor([selectButton]() {
            return selectButton->isEnabled();
        }, 5000);

        mouseClick(selectButton);
        int currentIdx;
        QTest::qWaitFor([tabs, &currentIdx]() {
            currentIdx = tabs->currentIndex();
            return currentIdx == 1;
        }, 5000);

        S_ASSERT(currentIdx == 1);
        TDEBUG("select button file tab - PASSED ");
    }

    void prepareSliceButtonClick() {
        TDEBUG("slice button prepare tab ");
        QPushButton* resliceButton = findWidget<QPushButton*>("prepareSlice");
        QLabel* imageGeneratorStatus = findWidget<QLabel*>("prepareImageGeneratorStatus");
        QLabel* siceStatus = findWidget<QLabel*>("prepareSliceStatus");
        QLabel* prepareNavigateCurrent = findWidget<QLabel*>("prepareNavigateCurrent");
        FileTab* fileTab = findWidget<FileTab*>("file");

        int baseThickness = printJob.getSelectedBaseLayerThickness();
        int bodyThickness = printJob.getSelectedBodyLayerThickness();
        int numberBase = printJob.getBaseLayerCount();
        int modelHeight = fileTab->modelSelection()->z.size * 1000;

        mouseClick(resliceButton);
        QTest::qWait(3000);
        QTest::qWaitFor([imageGeneratorStatus, siceStatus]() {
            return imageGeneratorStatus->text() == "idle" && siceStatus->text() == "idle";
        }, 500000);

        S_ASSERT(siceStatus->text() == "idle");
        S_ASSERT(imageGeneratorStatus->text() == "idle");
        QRegExp rx("(\\d+)\\/(\\d+)");
        S_ASSERT(rx.indexIn(prepareNavigateCurrent->text()) > -1);
        S_ASSERT(rx.cap(1).toInt() > 0);
        S_ASSERT(rx.cap(2).toInt() > 0);

        int overallCount = ceil((
            (float)modelHeight -
            (float)(numberBase * baseThickness)) / bodyThickness) +
            (float)numberBase;

        TDEBUG("overallCount: %d, modelHeight: %d, numberBase: %d, baseThickness: %d, bodyThickness: %d",
              overallCount, modelHeight, numberBase, baseThickness, bodyThickness);
        S_ASSERT(overallCount == rx.cap(2).toInt());

        TDEBUG("slice button prepare tab - PASSED");
    }

    void printContinueButtonClick() {
        TDEBUG("continue button print tab ");
        QPushButton* printContinue = findWidget<QPushButton*>("printPrint");
        QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
        QPushButton* stopButton = findWidget<QPushButton*>("statusStop");
        QLabel* statusDispensePrintSolution = findWidget<QLabel*>("statusDispensePrintSolution");

        mouseClick(printContinue);
        QTest::qWait(1000);

        S_ASSERT(statusStartPrint->isEnabled());
        S_ASSERT(stopButton->isEnabled());
        S_ASSERT(statusDispensePrintSolution->text() != "");
        TDEBUG("continue button print tab - PASSED");
    }

    void statusStartPrint() {
        TDEBUG("start print button status tab ");

        QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
        QPushButton* statusReprint = findWidget<QPushButton*>("statusReprint");
        QPushButton* statusPause = findWidget<QPushButton*>("statusPause");
        QLabel* statusModelFileName = findWidget<QLabel*>("statusModelFileName");
        QLabel* currentLayerDisplay = findWidget<QLabel*>("statusCurrentLayerDisplay");
        QRegExp rx ("Printing layer (%1) of (%2)");

        mouseClick(statusStartPrint);

        QTest::qWait(10000);

        TDEBUG(QString ( "'" % statusModelFileName->text() % "' '" % printJob.getModelFilename() % "'").toUtf8().data());

        S_ASSERT(statusModelFileName->text().endsWith(GetFileBaseName(printJob.getModelFilename())));

        QTest::qWaitFor([currentLayerDisplay]() {
            QRegExp rx ("(\\d+) of (\\d+)");
            if(rx.indexIn(currentLayerDisplay->text()) > -1) {

                return false;
            }

            return rx.cap(1).toInt() == rx.cap(2).toInt();
        }, 500000);

        QTest::qWait(10000);

        S_ASSERT(statusReprint->isEnabled());
        S_ASSERT(!statusPause->isEnabled());
        TDEBUG("start print button status tab - PASSED ");
    }

    void prepareClickPrepare() {
        TDEBUG("prepare button prepare tab ");

        QPushButton* prepareButton = findWidget<QPushButton*>("preparePrepare");
        QProgressBar* progressBar = findWidget<QProgressBar*>("preparePrepareProgress");

        mouseClick(prepareButton);
        QTest::qWait(1000);
        QTest::qWaitFor([progressBar]() {
            return !progressBar->isVisible();
        }, 50000);
        QTest::qWait(1000);
        S_ASSERT(prepareButton->isEnabled());
        S_ASSERT(prepareButton->text() == "Continue...");

        TDEBUG("prepare button prepare tab - PASSED ");
    }

    void prepareClickContinue() {
        TDEBUG("continue button prepare tab ");

        QPushButton* prepareButton = findWidget<QPushButton*>("preparePrepare");
        QProgressBar* progressBar = findWidget<QProgressBar*>("preparePrepareProgress");
        mouseClick(prepareButton);
        QTest::qWait(1000);
        QTest::qWaitFor([progressBar]() {
            return !progressBar->isVisible();
        }, 50000);
        QTest::qWait(1000);
        TDEBUG("continue button prepare tab - PASSED ");
    }

    void setCustomThickness() {
        TDEBUG("set custom thickness prepare tab ");

        QRadioButton* customThickness = findWidget<QRadioButton*>("prepareLayerThicknessCustom");
        QPushButton* resliceButton = findWidget<QPushButton*>("prepareSlice");
        mouseClick(customThickness);
        QTest::qWait(1000);

        QWidgetList widgetList = QApplication::topLevelWidgets();
        ThicknessWindow* thicknessWindow = findWidgetInTopLevel<ThicknessWindow*>("prepareThicknessWindow");
        ParamSlider* thicknessBase = findWidget<ParamSlider*>(thicknessWindow, "thicknessBaseLayerThickness");
        ParamSlider* thicknessBody = findWidget<ParamSlider*>(thicknessWindow, "thicknessBodyLayerThickness");
        ParamSlider* baseCount = findWidget<ParamSlider*>(thicknessWindow, "thicknessBaseLayerCount");
        QPushButton* thicknessOk = findWidget<QPushButton*>(thicknessWindow, "thicknessOk");

        auto random = QRandomGenerator::system();
        int tBody = random->bounded(1, 5) * 10;
        int tBase = tBody * random->bounded(1, (int)(100 / tBody));
        int tCount = random->bounded(1, 20);
        thicknessBody->setValue(tBody);
        thicknessBase->setValue(tBase);
        baseCount->setValue(tCount);

        mouseClick(thicknessOk);
        QTest::qWait(2000);

        S_ASSERT(printJob.getSelectedBaseLayerThickness() == tBase);
        S_ASSERT(printJob.getSelectedBodyLayerThickness() == tBody);
        S_ASSERT(customThickness->isEnabled());
        S_ASSERT(resliceButton->text() == "Custom reslice..." || resliceButton->text() == "Custom slice...");
        S_ASSERT(resliceButton->isEnabled());

        TDEBUG("set custom thickness prepare tab - PASSED");
    }

    void setupExposureTime() {
        TDEBUG("set exposure time print tab ");

        ParamSlider* bodyCoarse = findWidget<ParamSlider*>("printAdvBodyExpoCorse");
        ParamSlider* bodyFine = findWidget<ParamSlider*>("printAdvBodyExpoFine");
        ParamSlider* baseCoarse = findWidget<ParamSlider*>("printAdvBaseExpoCorse");
        ParamSlider* baseFine = findWidget<ParamSlider*>("printAdvBaseExpoFine");
        QLabel* statusLabel = findWidget<QLabel*>("statusDispensePrintSolution");
        Spoiler* printAdvancedExpoTime = findWidget<Spoiler*>("printAdvancedExpoTime");

        if(!printAdvancedExpoTime->isCollapsed())
            mouseClick(&printAdvancedExpoTime->toggleButton());

        QTest::qWait(2000);

        auto random = QRandomGenerator::system();
        int bodyCoarseValue = random->bounded(1, 10) * 1000;
        int bodyFineValue = random->bounded(0, 20) * 50;

        int baseCoarseValue = random->bounded(1, 10) * 1000;
        int baseFineValue = random->bounded(0, 20) * 50;

        int baseSum = baseCoarseValue + baseFineValue;
        int bodySum = bodyCoarseValue + bodyFineValue;

        bodyCoarse->setValue(bodyCoarseValue);
        bodyCoarse->onValueChanged(bodyCoarseValue);

        bodyFine->setValue(bodyFineValue);
        bodyFine->onValueChanged(bodyFineValue);

        baseCoarse->setValue(baseCoarseValue);
        baseCoarse->onValueChanged(baseCoarseValue);

        baseFine->setValue(baseFineValue);
        baseFine->onValueChanged(baseFineValue);

        QTest::qWait(2000);

        printContinueButtonClick();

        QRegularExpression rx("(\\d+(\\.\\d+)?) sec");
        auto iter = rx.globalMatch(statusLabel->text());
        S_ASSERT(iter.hasNext());

        auto next = iter.next();
        int capBase = (int)(next.captured(1).toDouble() * 1000);

        TDEBUG("capBase = %d", capBase);
        TDEBUG("baseSum = %d", baseSum);

        S_ASSERT(capBase == baseSum);

        next = iter.next();
        int capBody = (int)(next.captured(1).toDouble() * 1000);

        S_ASSERT(capBody == bodySum);
        TDEBUG("set exposure time print tab - PASSED ");
    }

    void pauseStopPrintTest() {
        TDEBUG("pause stop status tab");

        QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
        QPushButton* pause = findWidget<QPushButton*>("statusPause");
        QPushButton* stop = findWidget<QPushButton*>("statusStop");
        QPushButton* reprint = findWidget<QPushButton*>("statusReprint");
        QLabel* statusLabel = findWidget<QLabel*>("statusDispensePrintSolution");

        mouseClick(statusStartPrint);

        QTest::qWaitFor([pause]() {
            return pause->isEnabled();
        }, 50000);

        mouseClick(pause);

        QTest::qWait(1000);

        S_ASSERT((pause->text() == "Pausing..."));

        QTest::qWaitFor([pause]() {
            return (pause->text() == "Resume");
        }, 50000);

        S_ASSERT((pause->text() == "Resume"));

        mouseClick(pause);

        QTest::qWaitFor([pause]() {
            return (pause->text() == "Pause");
        }, 50000);

        S_ASSERT((pause->text() == "Pause"));

        QTest::qWait(1000);
        mouseClick(stop);

        QTest::qWaitFor([reprint, stop]() {
            return reprint->isVisible() && !stop->isVisible();
        }, 200000);

        mouseClick(reprint);

        S_ASSERT(!reprint->isVisible() && stop->isVisible());
        S_ASSERT(statusLabel->isVisible());


        TDEBUG("pause stop status tab - PASSED");
    }

    void setupBasicExposureTime() {
        TDEBUG("baisc exposure time setup");

        ParamSlider* bodyExpoSlider = findWidget<ParamSlider*>("printBodyExposureTime");

        bodyExpoSlider->setValue(1000);
        bodyExpoSlider->onValueChanged(1000);

        QTest::qWait(1000);
        TDEBUG("baisc exposure time setup - PASSED");
    }

    void tilingSwitchToTilingWindow() {
        TDEBUG("switch to tiling tab");

        QPushButton* setupTiling = findWidget<QPushButton*>("tilingSetupTiling");
        QPushButton* tilingSetupExpoTime = findWidget<QPushButton*>("tilingSetupExpoTime");
        QPushButton* tilingConfirm = findWidget<QPushButton*>("tilingConfirm");
        ParamSlider* tilingSpace = findWidget<ParamSlider*>("tilingSpace");
        ParamSlider* tilingCount = findWidget<ParamSlider*>("tilingCount");
        QTabWidget* tabs = findWidget<QTabWidget*>("tabWidget");
        QLabel* fileNameLabel = findWidget<QLabel*>("tilingFileName");

        tabs->setCurrentIndex(2);

        S_ASSERT(setupTiling->isEnabled());
        S_ASSERT(!tilingSetupExpoTime->isEnabled());
        S_ASSERT(!tilingConfirm->isEnabled());
        S_ASSERT(!tilingSpace->isEnabled());
        S_ASSERT(!tilingCount->isEnabled());
        S_ASSERT(fileNameLabel->text() == "");

        TDEBUG("switch to tiling tab - PASSED");
    }

    void tilingSetupTilingButton() {
        TDEBUG("setup tiling button click");


        QPushButton* setupTiling = findWidget<QPushButton*>("tilingSetupTiling");
        QPushButton* tilingSetupExpoTime = findWidget<QPushButton*>("tilingSetupExpoTime");
        QPushButton* tilingConfirm = findWidget<QPushButton*>("tilingConfirm");
        ParamSlider* tilingSpace = findWidget<ParamSlider*>("tilingSpace");
        ParamSlider* tilingCount = findWidget<ParamSlider*>("tilingCount");
        QLabel* fileNameLabel = findWidget<QLabel*>("tilingFileName");

        mouseClick(setupTiling);

        QTest::qWait(1000);

        S_ASSERT(!setupTiling->isEnabled());
        S_ASSERT(tilingSetupExpoTime->isEnabled());
        S_ASSERT(tilingConfirm->isEnabled());
        S_ASSERT(tilingSpace->isEnabled());
        S_ASSERT(tilingCount->isEnabled());
        S_ASSERT(fileNameLabel->text() == GetFileBaseName(printJob.getModelFilename()));

        TDEBUG("setup tiling button click - PASSED");
    }

    /* returns expected max tiles w count */
    int tilingSetSpacing() {
        TDEBUG("set tiling spacing");

        ParamSlider* tilingSpace = findWidget<ParamSlider*>("tilingSpace");
        ParamSlider* tilingCount = findWidget<ParamSlider*>("tilingCount");
        QLabel* tilitngLayerImage =  findWidget<QLabel*>("tilingCurrentLayerImage");
        auto random = QRandomGenerator::system();
        int spacing = random->bounded(1, 10);
        tilingSpace->setValue(spacing);

        QTest::qWait(3000);

        QPixmap* pixmap = new QPixmap(QString("%1/%2").arg(printJob.getLayerDirectory(0))
            .arg(printJob.getLayerFileName(0)));

        int areaWidth = tilitngLayerImage->width();
        int areaHeight = tilitngLayerImage->width();

        double wRatio = (static_cast<double>(areaWidth)) / ProjectorWindowSize.width();
        double hRatio = (static_cast<double>(areaHeight)) / ProjectorWindowSize.height();

        pixmap = new QPixmap(pixmap->scaled(static_cast<int>(pixmap->width() * wRatio),
                                            static_cast<int>(pixmap->height() * hRatio)));
        int pixmapWidth = pixmap->width();

        int wCount=0;
        int space = static_cast<int>(tilingSpace->getValue() / ProjectorPixelSize * wRatio);

        for (int i = static_cast<int>(TilingMargin * wRatio);
             i < (areaWidth - (TilingMargin * wRatio)); i += pixmapWidth, wCount++) {
            if (wCount > 0)
                i+=space;
        }
        wCount--;

        TDEBUG("Tiling max count slider=%d | calculated wCount=%d | space=%d | pixmapWidth=%d | wRatio = %f",
               tilingCount->getMaxValue(),
               wCount,
               tilingSpace->getValue(),
               pixmapWidth,
               wRatio
        );

        S_ASSERT(tilingSpace->getValue() == spacing);
        S_ASSERT(tilingCount->getMaxValue() == wCount);
        S_ASSERT(tilingCount->getMinValue() == 1);

        TDEBUG("set tiling spacing - PASSED");

        return wCount;
    }

    void tilingSetCount(int maxWCount) {
        TDEBUG("set tiling count");

        ParamSlider* tilingCount = findWidget<ParamSlider*>("tilingCount");
        auto random = QRandomGenerator::system();
        int count = random->bounded(1, maxWCount);

        dispatchToMainThread([tilingCount, count]() {
            tilingCount->setValue(count);
        });

        QTest::qWait(3000);

        S_ASSERT(tilingCount->getValue() == count);

        TDEBUG("set tiling count - PASSED");
    }

    void tilingSetExposureTime() {
        TDEBUG("set tiling set exposure time");
        QPushButton* tilingSetupExpoTime = findWidget<QPushButton*>("tilingSetupExpoTime");
        mouseClick(tilingSetupExpoTime);

        TilingExpoTimePopup* expoTimePopup = findWidgetInTopLevel<TilingExpoTimePopup*>("tilingExpoTimePopup");
        S_ASSERT(expoTimePopup->isVisible());

        ParamSlider* tilingExpoMinBase = findWidget<ParamSlider*>(expoTimePopup, "tilingExpoMinBase");
        ParamSlider* tilingExpoStepBase = findWidget<ParamSlider*>(expoTimePopup, "tilingExpoStepBase");
        ParamSlider* tilingExpoMinBody = findWidget<ParamSlider*>(expoTimePopup, "tilingExpoMinBody");
        ParamSlider* tilingExpoStepBody = findWidget<ParamSlider*>(expoTimePopup, "tilingExpoStepBody");
        QPushButton* tilingOkButton = findWidget<QPushButton*>(expoTimePopup, "tilingExpoOk");
        QLabel* tilingMinExposureBaseLabel = findWidget<QLabel*>("tilingMinExposureBase");
        QLabel* tilingStepBaseLabel = findWidget<QLabel*>("tilingStepBase");
        QLabel* tilingMinExposureBodyLabel = findWidget<QLabel*>("tilingMinExposureBody");
        QLabel* tilingStepBodyLabel = findWidget<QLabel*>("tilingStepBody");

        auto random = QRandomGenerator::system();
        int minBase = random->bounded(1, 40);
        int stepBase = random->bounded(1, 16);
        int minBody = random->bounded(1, 16);
        int stepBody = random->bounded(1, 16);

        dispatchToMainThread([
            tilingExpoMinBase, minBase,
            tilingExpoStepBase, stepBase,
            tilingExpoMinBody, minBody,
            tilingExpoStepBody, stepBody
        ]() {
            tilingExpoMinBase->setValue(minBase);
            tilingExpoStepBase->setValue(stepBase);
            tilingExpoMinBody->setValue(minBody);
            tilingExpoStepBody->setValue(stepBody);
        });

        QTest::qWait(1000);

        mouseClick(tilingOkButton);

        QTest::qWait(1000);

        QRegularExpression rx("(\\d+(\\.\\d+)?)s");
        auto iter = rx.globalMatch(tilingMinExposureBaseLabel->text());
        auto next = iter.next();

        S_ASSERT(next.captured(1) == QString::number((double)minBase / 4));

        iter = rx.globalMatch(tilingStepBaseLabel->text());
        next = iter.next();
        S_ASSERT(next.captured(1) == QString::number((double)stepBase / 4));

        iter = rx.globalMatch(tilingMinExposureBodyLabel->text());
        next = iter.next();
        S_ASSERT(next.captured(1) == QString::number((double)minBody / 4));

        iter = rx.globalMatch(tilingStepBodyLabel->text());
        next = iter.next();
        S_ASSERT(next.captured(1) == QString::number((double)stepBody / 4));

        TDEBUG("set tiling set exposure time - PASSED");
    }

    void tilingCreateTiles() {
        TDEBUG("create tiles click");

        QPushButton* tilingConfirm = findWidget<QPushButton*>("tilingConfirm");
        QTabWidget* tabs = findWidget<QTabWidget*>("tabWidget");

        mouseClick(tilingConfirm);

        QTest::qWaitFor([tabs]() {
            return tabs->currentIndex() == 1;
        }, 50000);

        S_ASSERT(tabs->currentIndex() == 1);

        TDEBUG("create tiles click - PASSED");
    }

    void switchToPrintProfileTab() {
        TDEBUG("switch to print profile tab");

        QTabWidget* tabs = findWidget<QTabWidget*>("tabWidget");
        QPushButton* profilesRename = findWidget<QPushButton*>("profilesRename");
        QPushButton* profilesOverwrite = findWidget<QPushButton*>("profilesOverwrite");
        QPushButton* profilesDelete = findWidget<QPushButton*>("profilesDelete");
        QPushButton* profilesLoad = findWidget<QPushButton*>("profilesLoad");
        QPushButton* profilesNew = findWidget<QPushButton*>("profilesNew");

        tabs->setCurrentIndex(6);

        S_ASSERT(profilesLoad->isEnabled());
        S_ASSERT(!profilesDelete->isEnabled());
        S_ASSERT(profilesNew->isEnabled());
        S_ASSERT(!profilesRename->isEnabled());
        S_ASSERT(profilesOverwrite->isEnabled());

        TDEBUG("switch to print profile tab - PASSED");
    }

    void createPrintProfile() {
        QPushButton* profilesNew = findWidget<QPushButton*>("profilesNew");
        mouseClick(profilesNew);

        InputDialog* inputDialog = findWidgetInTopLevel<InputDialog*>("inputdialog");
        Keyboard* keyboard = findWidget<Keyboard*>(inputDialog, "keyboard");
        QPushButton* okButton = findWidget<QPushButton*>("keyboardOk");
        QLineEdit* inputLine = findWidget<QLineEdit*>("keyboardInput");
        QStandardItemModel* profilesModel = findWidget<QStandardItemModel*>("profilesModel");


        S_ASSERT(inputDialog->isVisible());
        QString test = getRandomPrintProfiletName();

        for(int i=0; i<test.length(); ++i) {
            QString c = test.at(i);
            key* k = keyboard->getKey(c);

            dispatchToMainThread([keyboard, k]() {
            QTest::mouseClick(keyboard, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier,
                              QPoint(k->getRect().x(), k->getRect().y()));
            });

            QTest::qWait(200);
        }

        //@todo add cancel button test
        S_ASSERT(inputLine->text() == test);
        mouseClick(okButton);
        QTest::qWait(200);
        auto items = profilesModel->findItems(test);
        S_ASSERT(items.length() == 1);

        clickOkMsgBoxTopLevel();

        QMessageBox* profilesNewConfirmation = findWidget<QMessageBox*>("profilesNewConfirmation");

    }


signals:
    void successed();
    void failed(QString message);
protected:
    QString _modelName;
};

#endif // ABSTRACTTEST_H
