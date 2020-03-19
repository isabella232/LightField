#include "paramslider.h"


void ParamSlider::init(QString name, QString unit, int startValue, int maxValue, int step, int minValue)
{
    QString valueLabel = QString::number(startValue) + QString(" ") + unit;
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );

    this->_nameLabel->setText(name);
    this->_valueLabel->setText(valueLabel);
    this->_valueLabel->setFont(boldFont);
    this->_slider->setValue(startValue);
    this->_valueLabel->setText(QString::number(_slider->value() / _factor) + " " + QString(_unit));
    this->_slider->setMaximum(maxValue);
    this->_slider->setMinimum(minValue);
    this->_slider->setSingleStep(step);
    this->_slider->setOrientation(Qt::Orientation::Horizontal);

    QWidget::connect(this->_slider, &QSlider::valueChanged, this, &ParamSlider::onvaluechanged);

    this->setLayout(
        WrapWidgetsInVBoxDM(
            WrapWidgetsInHBox(_nameLabel, nullptr, _valueLabel),
            _slider
        )
    );

    _unit = unit;
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

void ParamSlider::onvaluechanged(int)
{
    debug( "+ ParamSlider::onvaluechanged %f\n", _slider->value() / _factor);

    this->_valueLabel->setText(QString::number(_slider->value() / _factor) + " " + QString(_unit));
    emit valuechanged();
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

ParamSlider::ParamSlider(QString name, int maxValue)
{
    this->init(name, "", 0, maxValue, 1, 0);
}

ParamSlider::~ParamSlider()
{ }
