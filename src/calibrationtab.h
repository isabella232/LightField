#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

class CalibrationTab: public QWidget {

    Q_OBJECT

public:

    CalibrationTab( QWidget* parent = nullptr );
    virtual ~CalibrationTab( ) override;

protected:

private:

    std::vector<QLabel*> _stepsStatus;

    QVBoxLayout*         _leftColumnLayout  { new QVBoxLayout };
    QPushButton*         _calibrateButton   { new QPushButton };

    QVBoxLayout*         _rightColumnLayout { new QVBoxLayout };
    QGroupBox*           _rightColumn       { new QGroupBox   };

    QGridLayout*         _layout            { new QGridLayout };

signals:

public slots:

protected slots:

private slots:

    void _calibrateButton_clicked( bool );

};

#endif // __CALIBRATION_H__
