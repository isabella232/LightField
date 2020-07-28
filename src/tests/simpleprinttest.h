#ifndef SIMPLEPRINTTEST_H
#define SIMPLEPRINTTEST_H

#include <QtWidgets>
#include <QtTest/QtTest>
#include "abstracttest.h"

class SimplePrintTest: public AbstractTest
{
public:
    SimplePrintTest();
    void start() override;

    QObject* findWidgetByNameRoot(QString name) {
        Window* mainWindow = (Window*)App::mainWindow();
        return  findWidgetByName((QObject*)mainWindow, name);
    }

    QObject* findWidgetByName(QObject* root, QString name) {
        foreach(QObject* widget, root->children()) {
            if(widget->objectName() == name) {
                return widget;
            } else {
                if(!widget) {
                    return nullptr;
                }

                QObject* result = findWidgetByName(widget, name);
                if(result != nullptr)
                    return result;
            }
        }

        return nullptr;
    }

    void mouseClick(QWidget* widget) {
        QRect rect = widget->geometry();
        QWindow* mainWindow = (QWindow*)App::mainWindow();

        QTest::mouseClick(mainWindow, Qt::LeftButton, 0, rect.center());
    }
};

#endif // SIMPLEPRINTTEST_H
