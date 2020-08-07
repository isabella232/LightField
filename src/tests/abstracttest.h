#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

#include <QtTest>
#include "../window.h"
#include "../filetab.h"
#include "../thicknesswindow.h"
#include "../canvas.h"
#include "../spoiler.h"

class AbstractTest: public QObject {
    Q_OBJECT
public:
    virtual QString testName() = 0;

    virtual void start() = 0;

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

        Q_ASSERT(window->isInitialized());
    }

    //selecting model on list
    void fileSelectModelOnList() {
        debug("============ TEST: file select model on list \n");
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

        Q_ASSERT(!canvas->objectName().isEmpty());
        Q_ASSERT(deleteButton->isEnabled());
        Q_ASSERT(selectButton->isEnabled());
        Q_ASSERT(fileViewSolid->isEnabled());
        Q_ASSERT(fileViewWireframe->isEnabled());

        QRegExp rx("\\d+\\.\\d+ mm × \\d+\\.\\d+ mm × \\d+\\.\\d+ mm, \\d+\\.\\d+ µL");
        Q_ASSERT(rx.indexIn(fileDimensions->text()) != -1);
        debug("============ TEST: file select model on list - PASSED \n");
    }

    void fileClickSelectButton() {
        debug("============ TEST: select button file tab \n");
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

        Q_ASSERT(currentIdx == 1);
        Q_ASSERT(resliceButton->isEnabled());
        debug("============ TEST: select button file tab - PASSED \n");
    }

    void prepareSliceButtonClick() {
        debug("============ TEST: slice button prepare tab \n");
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
        }, 50000);

        Q_ASSERT(siceStatus->text() == "idle");
        Q_ASSERT(imageGeneratorStatus->text() == "idle");
        QRegExp rx("(\\d+)\\/(\\d+)");
        debug("%d", rx.indexIn(prepareNavigateCurrent->text()));
        Q_ASSERT(rx.indexIn(prepareNavigateCurrent->text()) > -1);
        Q_ASSERT(rx.cap(1).toInt() > 0);
        Q_ASSERT(rx.cap(2).toInt() > 0);

        int overallCount = ((modelHeight - (numberBase * baseThickness)) / bodyThickness) + numberBase;
        debug("============ TEST: overallCount: %d, modelHeight: %d, numberBase: %d, baseThickness: %d, bodyThickness: %d",
              overallCount, modelHeight, numberBase, baseThickness, bodyThickness);
        Q_ASSERT(overallCount == rx.cap(2).toInt());

        debug("============ TEST: slice button prepare tab - PASSED\n");
    }

    void printContinueButtonClick() {
        debug("============ TEST: continue button print tab \n");
        QPushButton* printContinue = findWidget<QPushButton*>("printPrint");
        QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
        QPushButton* stopButton = findWidget<QPushButton*>("statusStop");
        QLabel* statusDispensePrintSolution = findWidget<QLabel*>("statusDispensePrintSolution");

        mouseClick(printContinue);
        QTest::qWait(1000);

        Q_ASSERT(statusStartPrint->isEnabled());
        Q_ASSERT(stopButton->isEnabled());
        Q_ASSERT(statusDispensePrintSolution->text() != "");
        debug("============ TEST: continue button print tab - PASSED\n");
    }

    void statusStartPrint() {
        debug("============ TEST: start print button status tab \n");

        QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
        QPushButton* statusReprint = findWidget<QPushButton*>("statusReprint");
        QPushButton* statusPause = findWidget<QPushButton*>("statusPause");
        QLabel* statusModelFileName = findWidget<QLabel*>("statusModelFileName");
        QLabel* currentLayerDisplay = findWidget<QLabel*>("statusCurrentLayerDisplay");
        QRegExp rx ("Printing layer (%1) of (%2)");

        mouseClick(statusStartPrint);

        QTest::qWait(10000);

        debug(QString ( "'" % statusModelFileName->text() % "' '" % printJob.getModelFilename() % "'").toUtf8().data());

        Q_ASSERT(statusModelFileName->text().endsWith(GetFileBaseName(printJob.getModelFilename())));

        QTest::qWaitFor([currentLayerDisplay]() {
            QRegExp rx ("(\\d+) of (\\d+)");
            if(rx.indexIn(currentLayerDisplay->text()) > -1) {

                return false;
            }

            return rx.cap(1).toInt() == rx.cap(2).toInt();
        }, 500000);

        QTest::qWait(10000);

        Q_ASSERT(statusReprint->isEnabled());
        Q_ASSERT(!statusPause->isEnabled());
        debug("============ TEST: start print button status tab - PASSED \n");
    }

    void prepareClickPrepare() {
        debug("============ TEST: prepare button prepare tab \n");

        QPushButton* prepareButton = findWidget<QPushButton*>("preparePrepare");
        QProgressBar* progressBar = findWidget<QProgressBar*>("preparePrepareProgress");
        mouseClick(prepareButton);
        QTest::qWait(1000);
        QTest::qWaitFor([progressBar]() {
            return !progressBar->isVisible();
        }, 50000);

        Q_ASSERT(prepareButton->isEnabled());
        Q_ASSERT(prepareButton->text() == "Continue...");

        debug("============ TEST: prepare button prepare tab - PASSED \n");
    }

    void prepareClickContinue() {
        debug("============ TEST: continue button prepare tab \n");

        QPushButton* prepareButton = findWidget<QPushButton*>("preparePrepare");
        QProgressBar* progressBar = findWidget<QProgressBar*>("preparePrepareProgress");
        mouseClick(prepareButton);
        QTest::qWait(1000);
        QTest::qWaitFor([progressBar]() {
            return !progressBar->isVisible();
        }, 50000);

        debug("============ TEST: continue button prepare tab - PASSED \n");
    }

    void setCustomThickness() {
        debug("============ TEST: set custom thickness prepare tab \n");

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
        int tBody = random->bounded(1, 10) * 10;
        int tBase = tBody * random->bounded(1, 100 / tBody);
        int tCount = random->bounded(1, 20);
        thicknessBody->setValue(tBody);
        thicknessBase->setValue(tBase);
        baseCount->setValue(tCount);

        mouseClick(thicknessOk);
        QTest::qWait(2000);

        Q_ASSERT(printJob.getSelectedBaseLayerThickness() == tBase);
        Q_ASSERT(printJob.getSelectedBodyLayerThickness() == tBody);
        Q_ASSERT(customThickness->isEnabled());
        Q_ASSERT(resliceButton->text() == "Custom reslice..." || resliceButton->text() == "Custom slice...");
        Q_ASSERT(resliceButton->isEnabled());

        debug("============ TEST: set custom thickness prepare tab \n");
    }

    void setupExposureTime() {
        debug("============ TEST: set exposure time print tab \n");

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
        Q_ASSERT(iter.hasNext());

        auto next = iter.next();
        int capBase = (int)(next.captured(1).toDouble() * 1000);

        Q_ASSERT(capBase == baseSum);

        next = iter.next();
        int capBody = (int)(next.captured(1).toDouble() * 1000);

        Q_ASSERT(capBody == bodySum);
        debug("============ TEST: set exposure time print tab - PASSED \n");
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

        Q_ASSERT((pause->text() == "Pausing..."));

        QTest::qWaitFor([pause]() {
            return (pause->text() == "Resume");
        }, 50000);

        Q_ASSERT((pause->text() == "Resume"));

        mouseClick(pause);

        QTest::qWait(1000);

        Q_ASSERT((pause->text() == "Pause"));

        QTest::qWait(1000);
        mouseClick(stop);
        //Q_ASSERT(stop->text() == "Stopping...");

        QTest::qWaitFor([reprint, stop]() {
            return reprint->isVisible() && !stop->isVisible();
        }, 20000);

        mouseClick(reprint);

        Q_ASSERT(!reprint->isVisible() && stop->isVisible());
        Q_ASSERT(statusLabel->isVisible());
    }

signals:
    void successed();
    void failed(QString message);
};

#endif // ABSTRACTTEST_H
