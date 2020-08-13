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
#include "../spoiler.h"
#include <iostream>

extern FILE* TestLog;

class AbstractTest: public QObject {
    Q_OBJECT
public:
    virtual QString testName() = 0;

    virtual void start() = 0;

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
        TDEBUG( "%d", arg1 );
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
    void fileSelectModelOnList() {
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
        QModelIndex index = model->index("/var/lib/lightfield/model-library/Menger_sponge_sample.stl");
        availableFilesListView->setCurrentIndex(model->index("/var/lib/lightfield/model-library/Menger_sponge_sample.stl"));
        availableFilesListView->clicked(index);

        QTest::qWait(1000);

        S_ASSERT(!canvas->objectName().isEmpty());
        S_ASSERT(deleteButton->isEnabled());
        S_ASSERT(selectButton->isEnabled());
        S_ASSERT(fileViewSolid->isEnabled());
        S_ASSERT(fileViewWireframe->isEnabled());

        QRegExp rx("\\d+\\.\\d+ mm × \\d+\\.\\d+ mm × \\d+\\.\\d+ mm, \\d+\\.\\d+ µL");
        S_ASSERT(rx.indexIn(fileDimensions->text()) != -1);
        TDEBUG("file select model on list - PASSED ");
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
        S_ASSERT(resliceButton->isEnabled());
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
        QTest::qWait(1000);
        QTest::qWaitFor([imageGeneratorStatus, siceStatus]() {
            return imageGeneratorStatus->text() == "idle" && siceStatus->text() == "idle";
        }, 500000);

        S_ASSERT(siceStatus->text() == "idle");
        S_ASSERT(imageGeneratorStatus->text() == "idle");
        QRegExp rx("(\\d+)\\/(\\d+)");
        S_ASSERT(rx.indexIn(prepareNavigateCurrent->text()) > -1);
        S_ASSERT(rx.cap(1).toInt() > 0);
        S_ASSERT(rx.cap(2).toInt() > 0);

        int overallCount = ((modelHeight - (numberBase * baseThickness)) / bodyThickness) + numberBase;
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

        S_ASSERT(capBase == baseSum);

        next = iter.next();
        int capBody = (int)(next.captured(1).toDouble() * 1000);

        S_ASSERT(capBody == bodySum);
        TDEBUG("set exposure time print tab - PASSED ");
    }

    void pauseStopPrintTest() {
        QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
        QPushButton* pause = findWidget<QPushButton*>("statusPause");
        QPushButton* stop = findWidget<QPushButton*>("statusStop");
        QPushButton* reprint = findWidget<QPushButton*>("statusReprint");
        QLabel* statusLabel = findWidget<QLabel*>("statusDispensePrintSolution");

        mouseClick(statusStartPrint);

        QTest::qWait(4000);

        mouseClick(pause);

        QTest::qWait(500);

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
    }

signals:
    void successed();
    void failed(QString message);
};

#endif // ABSTRACTTEST_H
