#include <QtCore>
#include <QtWidgets>
#include "utils.h"
#include "paramslider.h"

void ParamSlider::init(QString name, QString unit, int startValue, int maxValue, int step,
    int minValue)
{
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );

    _unit = unit;
    this->_nameLabel->setText(name);
    this->_valueLabel->setFont(boldFont);
    this->_slider->setValue(startValue);

    this->_valueLabel->setText( QString::number(startValue / _factor) + " " + QString(_unit) );
    this->_slider->setMaximum(maxValue);
    this->_slider->setMinimum(minValue);
    this->_slider->setSingleStep(step);
    this->_slider->setTickInterval( step );
    this->_slider->setPageStep( step );
    this->_slider->setOrientation(Qt::Orientation::Horizontal);
    this->setStyleSheet( "padding: 0px 0px 0px 0px; " );

    QWidget::connect(this->_slider, &QSlider::valueChanged, this, &ParamSlider::onValueChanged);

    this->setLayout(
        WrapWidgetsInVBoxDM(
            nullptr,
            WrapWidgetsInHBox(_nameLabel, nullptr, _valueLabel),
            nullptr,
            _slider,
            nullptr
        )
    );
}

double ParamSlider::getValueDouble()
{
    return _slider->value() / _factor;
}

int ParamSlider::getValue()
{
    return _slider->value();
}

void ParamSlider::setValue(int value)
{
    _slider->setValue(value);
}

void ParamSlider::setValueDouble(double value)
{
   _slider->setValue(value * _factor);
}

void ParamSlider::setMaxValue(int value)
{
   _slider->setMaximum( value );

   if(_slider->value() > value)
   {
       _slider->setValue( value );
   }
}

void ParamSlider::setMinValue(int value)
{
   _slider->setMinimum( value );

   if(_slider->value() < value)
   {
       _slider->setValue( value );
   }
}

void ParamSlider::setStep(int step)
{
    int stepRemainder = _slider->value() % step;
    int newValue;

   _slider->setSingleStep( step );
   _slider->setTickInterval( step );
   _slider->setPageStep( step );

   if( stepRemainder )
   {
       if (_slider->value() - stepRemainder < _slider->minimum()) {
           newValue = _slider->value() - stepRemainder + step;
           if (newValue > _slider->maximum()) {
               _slider->setValue(_slider->maximum());
           } else {
               _slider->setValue(newValue);
           }
       } else {
           _slider->setValue(_slider->value() - stepRemainder);
       }
   }
}

void ParamSlider::onValueChanged(int)
{
    int remainder = _slider->value() % _slider->singleStep();
    if ( remainder ) {
        if (remainder > _slider->singleStep() / 2) {
            _slider->setValue(_slider->value() + _slider->singleStep() - remainder);
        } else {
            _slider->setValue(_slider->value() - remainder);
        }
    }
    this->_valueLabel->setText( QString::number(_slider->value() / _factor) + " " + QString(_unit) );
    emit valueChanged();
}

ParamSlider::ParamSlider(QString name, QString unit, int startValue, int maxValue, int step, int minValue, double factor)
{
    this->_factor = factor;
    this->init(name, unit, startValue, maxValue, step, minValue);
}

ParamSlider::ParamSlider(QString name, QString unit, int startValue, int maxValue, int step, int minValue)
{
    this->init(name, unit, startValue, maxValue, step, minValue);
}

ParamSlider::ParamSlider(QString name, QString unit, int startValue, int maxValue, int step)
{
    this->init(name, unit, startValue, maxValue, step, 0);
}

ParamSlider::ParamSlider(QString name, int maxValue)
{
    this->init(name, "", 0, maxValue, 1, 0);
}

void   ParamSlider::setFontColor(QString font)
{
    this->_nameLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(font));
    this->_valueLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(font));
}

ParamSlider::~ParamSlider()
{ }
