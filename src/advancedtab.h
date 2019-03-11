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

    PrintJob*     _printJob           { };
    PrintManager* _printManager       { };
    Shepherd*     _shepherd           { };

    QLabel*       _currentTemperature { new QLabel };
    QLabel*       _targetTemperature  { new QLabel };
    QLabel*       _pwm                { new QLabel };

    QTimer*       _timer              { new QTimer };

signals:

public slots:

    void setPrintManager( PrintManager* printManager );
    void setShepherd( Shepherd* shepherd );

    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void printManager_printStarting( );
    void printManager_printComplete( bool const success );
    void printManager_printAborted( );

protected slots:

private slots:

    void timer_pollTemperature( );

};

#endif // __ADVANCEDTAB_H__
