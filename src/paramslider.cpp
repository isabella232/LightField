#include "paramslider.h"

void ParamSlider::init(QString name, QString unit, int startValue, int maxValue, int step)
{
    QString valueLabel = QString::number(startValue) + QString(" ") + unit;
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );

    this->_nameLabel->setText(name);
    this->_valueLabel->setText(valueLabel);
    this->_valueLabel->setFont(boldFont);
    this->_slider->setValue(startValue);
    this->_slider->setMaximum(maxValue);
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

int ParamSlider::getValue()
{
    return _slider->value();
}

void ParamSlider::onvaluechanged(int value)
{
    this->_valueLabel->setText(QString::number(_slider->value()) + " " + QString(_unit));
}

ParamSlider::ParamSlider(QString name, QString unit, int startValue, int maxValue, int step)
{
    this->init(name, unit, startValue, maxValue, step);
}

ParamSlider::ParamSlider(QString name, int maxValue)
{
    this->init(name, "", 0, maxValue, 1);
}

ParamSlider::~ParamSlider()
{ }

