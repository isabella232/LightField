#ifndef __PARAMSLIDER_H__
#define __PARAMSLIDER_H__

#include "tabbase.h"

class ParamSlider: public QGroupBox {
    Q_OBJECT
    private:
        QSlider*     _slider     { new QSlider };
        QLabel*      _nameLabel  { new QLabel };
        QLabel*      _valueLabel { new QLabel };
        QString      _unit;

        void init(QString name, QString unit, int startValue, int maxValue, int step);

    public:
        ParamSlider(QString name, QString unit, int startValue, int maxValue, int step);
        ParamSlider(QString name, int maxValue);
        ~ParamSlider();

        int getValue();
        void setValue(int value);

    signals:
        void valuechanged();
    public slots:
        void onvaluechanged(int value);
};

#endif // __PARAMSLIDER_H__
