#include <QtCore>
#include <QtWidgets>
#include "thicknesswindow.h"
#include "constants.h"
#include "utils.h"

ThicknessWindow::ThicknessWindow(bool initValues, QWidget *parent):
    QDialog(parent)
{
    QVBoxLayout *layout;
    QHBoxLayout *buttons = new QHBoxLayout;
    auto origFont = font();
    auto font22pt = ModifyFont(origFont, LargeFontSize);

    this->setModal( true );

    _ok = new QPushButton("OK");
    _ok->setFont(font22pt);
    QWidget::connect(_ok, &QPushButton::clicked, this, &ThicknessWindow::ok_clicked);

    _cancel = new QPushButton("Cancel");
    _cancel->setFont(font22pt);

    QWidget::connect(_cancel, &QPushButton::clicked, this, &ThicknessWindow::cancel_clicked);

    if(!initValues) {
        int step = static_cast<int>( ceil(100.0 / printJob.getSelectedBodyLayerThickness()) );

        _baseLayerCount->setValue(printJob.getBaseLayerCount());

        _baseLayerThickness->setMinValue(printJob.getSelectedBodyLayerThickness());
        if (100 % printJob.getSelectedBodyLayerThickness()) {
            _baseLayerThickness->setMaxValue((step - 1) * printJob.getSelectedBodyLayerThickness());
        } else {
            _baseLayerThickness->setMaxValue(step * printJob.getSelectedBodyLayerThickness());
        }
        _baseLayerThickness->setStep(printJob.getSelectedBodyLayerThickness());
        _baseLayerThickness->setValue(printJob.getSelectedBaseLayerThickness());

        _bodyLayerThickness->setValue(printJob.getSelectedBodyLayerThickness());
    } else {
        _bodyLayerThickness->setValue( 20 );

        _baseLayerCount->setValue( 2 );

        _baseLayerThickness->setStep( 20 );
        _baseLayerThickness->setMinValue( 20 );
        _baseLayerThickness->setMaxValue( 100 );
        _baseLayerThickness->setValue( 20 );
    }


    if (printJob.getDirectoryMode()) {
        QObject::connect(_baseLayerThickness, &ParamSlider::valueChanged, [=]() {
            _bodyLayerThickness->setValue(_baseLayerThickness->getValue());
        });

        QObject::connect(_bodyLayerThickness, &ParamSlider::valueChanged, [=]() {
            _baseLayerThickness->setValue(_bodyLayerThickness->getValue());
        });
    } else {
        QObject::connect(_bodyLayerThickness, &ParamSlider::valueChanged, [=]() {
            _baseLayerThickness->setMinValue( _bodyLayerThickness->getValue() );

            int step = static_cast<int>( ceil(100.0 / _bodyLayerThickness->getValue()) );

            if ( 100 % _bodyLayerThickness->getValue() ) {
                _baseLayerThickness->setMaxValue( (step - 1) * _bodyLayerThickness->getValue() );
            } else {
                _baseLayerThickness->setMaxValue( step * _bodyLayerThickness->getValue() );
            }
            _baseLayerThickness->setStep( _bodyLayerThickness->getValue() );
        });
    }

    buttons->addWidget(_ok);
    buttons->addWidget(_cancel);
    layout = WrapWidgetsInVBox(
        _baseLayerCount,
        _bodyLayerThickness,
        _baseLayerThickness);

    layout->addLayout(buttons);
    setLayout(layout);
}

void ThicknessWindow::ok_clicked(bool) {
    printJob.setBaseLayerCount(_baseLayerCount->getValue());
    printJob.setSelectedBaseLayerThickness(_baseLayerThickness->getValue());
    printJob.setSelectedBodyLayerThickness(_bodyLayerThickness->getValue());
    done(QDialog::Accepted);
}

void ThicknessWindow::cancel_clicked(bool) {
    reject();
}
