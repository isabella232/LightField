#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

#include <QtTest>
#include "../window.h"
#include "../filetab.h"
#include "../canvas.h"

class AbstractTest: public QObject {
    Q_OBJECT
public:
    static QString testName;

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
        //selecting model on list
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
    }

    void fileClickSelectButton() {
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
    }

    void prepareSliceButtonClick() {
        QPushButton* resliceButton = findWidget<QPushButton*>("prepareSlice");
        QLabel* imageGeneratorStatus = findWidget<QLabel*>("prepareImageGeneratorStatus");
        QLabel* siceStatus = findWidget<QLabel*>("prepareSliceStatus");
        QLabel* prepareNavigateCurrent = findWidget<QLabel*>("prepareNavigateCurrent");

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
    }

    void printContinueButtonClick() {
        QPushButton* printContinue = findWidget<QPushButton*>("printPrint");
        QPushButton* statusStartPrint = findWidget<QPushButton*>("statusStartThePrint");
        QPushButton* stopButton = findWidget<QPushButton*>("statusStop");
        QLabel* statusDispensePrintSolution = findWidget<QLabel*>("statusDispensePrintSolution");

        mouseClick(printContinue);
        QTest::qWait(1000);

        Q_ASSERT(statusStartPrint->isEnabled());
        Q_ASSERT(stopButton->isEnabled());
        Q_ASSERT(statusDispensePrintSolution->text() != "");
    }

    void statusStartPrint() {
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
    }

    void prepareClickPrepare() {

    }

    void prepareClickContinue() {

    }

signals:
    void successed();
    void failed(QString message);
};

#endif // ABSTRACTTEST_H
