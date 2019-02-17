#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

class Shepherd;

class CalibrationTab: public QWidget {

    Q_OBJECT

public:

    CalibrationTab( QWidget* parent = nullptr );
    virtual ~CalibrationTab( ) override;

    Shepherd* shepherd( ) const { return _shepherd; }

protected:

private:

    std::vector<QLabel*> _stepsStatus;

    Shepherd*            _shepherd;

    //
    // Left column
    //

    QVBoxLayout*         _leftColumnLayout       { new QVBoxLayout  };
    QPushButton*         _calibrateButton        { new QPushButton  };

    //
    // Right column
    //

    QLabel*              _calibrationMessage     { new QLabel       };
    QProgressBar*        _calibrationProgress    { new QProgressBar };
    QPushButton*         _continueButton         { new QPushButton  };

    QVBoxLayout*         _calibrationStepsLayout { new QVBoxLayout  };

    QVBoxLayout*         _rightColumnLayout      { new QVBoxLayout  };
    QGroupBox*           _rightColumn            { new QGroupBox    };

    QGridLayout*         _layout                 { new QGridLayout  };

signals:

    void calibrationComplete( bool const success );

public slots:

    void setShepherd( Shepherd* shepherd );

protected slots:

private slots:

    void _calibrateButton_clicked( bool );
    void _sendHome_complete( bool const success );
    void _adjustBuildPlatform_complete( bool );
    void _sendResinLoadMove_complete( bool const success );
    void _loadPrintSolution_complete( bool );
    void _sendExtend_complete( bool const success );

};

#endif // __CALIBRATION_H__
