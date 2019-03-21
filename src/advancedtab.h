#ifndef __ADVANCEDTAB_H__
#define __ADVANCEDTAB_H__

class PrintJob;
class PrintManager;
class Shepherd;

class AdvancedTab: public QWidget {

    Q_OBJECT

public:

    AdvancedTab( QWidget* parent = nullptr );
    virtual ~AdvancedTab( ) override;

    Shepherd* shepherd( ) const { return _shepherd; }

protected:

private:

    PrintJob*     _printJob                { };
    PrintManager* _printManager            { };
    Shepherd*     _shepherd                { };

    QLabel*       _currentTemperatureLabel { new QLabel };
    QLabel*       _targetTemperatureLabel  { new QLabel };
    QLabel*       _pwmLabel                { new QLabel };
    QLabel*       _zPositionLabel          { new QLabel };

    QLabel*       _currentTemperature      { new QLabel };
    QLabel*       _targetTemperature       { new QLabel };
    QLabel*       _pwm                     { new QLabel };
    QLabel*       _zPosition               { new QLabel };

    QTimer*       _timer                   { new QTimer };

    void _pauseTimer( );
    void _resumeTimer( );

signals:

public slots:

    void setPrintManager( PrintManager* printManager );
    void setShepherd( Shepherd* shepherd );

    void printer_positionReport( double const px, double const py, double const pz, double const pe, double const cx, double const cy, double const cz );
    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void printManager_printStarting( );
    void printManager_printComplete( bool const success );
    void printManager_printAborted( );

protected slots:

private slots:

    void timer_pollTemperature( );

};

#endif // __ADVANCEDTAB_H__
