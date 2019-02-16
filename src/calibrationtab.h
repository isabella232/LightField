#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

class CalibrationTab: public QWidget {

    Q_OBJECT

public:

    CalibrationTab( QWidget* parent = nullptr );
    virtual ~CalibrationTab( ) override;

protected:

private:

    QHBoxLayout* _layout { new QHBoxLayout };

signals:

public slots:

protected slots:

private slots:

};

#endif // __CALIBRATION_H__
