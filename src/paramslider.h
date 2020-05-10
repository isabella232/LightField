#ifndef __PARAMSLIDER_H__
#define __PARAMSLIDER_H__

#include <QtCore>
#include <QtWidgets>

class ParamSlider: public QGroupBox
{
    Q_OBJECT

private:
    QSlider*     _slider     { new QSlider };
    QLabel*      _nameLabel  { new QLabel };
    QLabel*      _valueLabel { new QLabel };
    QString      _unit;
    double       _factor     { 1.0L };

    void init(QString name, QString unit, int startValue, int maxValue, int step, int minValue);

public:
    ParamSlider(QString name, QString unit, int startValue, int maxValue, int step);
    ParamSlider(QString name, QString unit, int startValue, int maxValue, int step, int minValue);
    ParamSlider(QString name, QString unit, int startValue, int maxValue, int step, int minValue, double factor);
    ParamSlider(QString name, int maxValue);
    ~ParamSlider();

    int    getValue();
    void   setValue(int value);
    double getValueDouble();
    void   setValueDouble(double value);
    void   setMinValue(int value);
    void   setStep(int step);
    void   setMaxValue(int value);

signals:
    void valueChanged();

public slots:
    void onValueChanged(int value);
};

#endif // __PARAMSLIDER_H__
