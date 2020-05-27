#ifndef __THICKNESSWINDOW_H__
#define __THICKNESSWINDOW_H__

#include <QtCore>
#include <QtWidgets>
#include "paramslider.h"
#include "printjob.h"

class ThicknessWindow: public QDialog
{
    Q_OBJECT

public:
    ThicknessWindow(QSharedPointer<PrintJob> job, QWidget *parent = nullptr);

protected:
    QSharedPointer<PrintJob> _printJob;
    QPushButton *_ok;
    QPushButton *_cancel;
    ParamSlider *_baseLayerCount { new ParamSlider("Number of Base Layers", "", 1, 20, 1, 0) };
    ParamSlider *_baseLayerThickness { new ParamSlider( "Base Layer Thickness", "µm", 100, 100, 100, 100) };
    ParamSlider *_bodyLayerThickness { new ParamSlider( "Body Layer Thickness", "µm", 10, 100, 10, 10) };

    void ok_clicked( bool );
    void cancel_clicked( bool );
};

#endif // __THICKNESSWINDOW_H__
