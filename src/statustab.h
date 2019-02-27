#ifndef __STATUSTAB_H__
#define __STATUSTAB_H__

class PrintJob;
class Shepherd;

class StatusTab: public QWidget {

    Q_OBJECT

public:

    StatusTab( QWidget* parent = nullptr );
    virtual ~StatusTab( ) override;

    bool isStopButtonEnabled( ) {
        return stopButton->isEnabled( );
    }

    Shepherd* shepherd( ) const { return _shepherd; }

protected:

    virtual void showEvent( QShowEvent* ) override;

private:

    QLabel*      printerStateLabel                 { new QLabel      };
    QLabel*      printerStateDisplay               { new QLabel      };
    QLabel*      projectorLampStateLabel           { new QLabel      };
    QLabel*      projectorLampStateDisplay         { new QLabel      };
    QLabel*      jobStateLabel                     { new QLabel      };
    QLabel*      jobStateDisplay                   { new QLabel      };
    QLabel*      currentLayerLabel                 { new QLabel      };
    QLabel*      currentLayerDisplay               { new QLabel      };
    QVBoxLayout* progressControlsLayout            { new QVBoxLayout };
    QWidget*     progressControlsContainer         { new QWidget     };
    QLabel*      currentLayerImage                 { new QLabel      };
    QGroupBox*   currentLayerImageGroup            { new QGroupBox   };
    QPushButton* stopButton                        { new QPushButton };
    QGridLayout* _layout                           { new QGridLayout };
    PrintJob*    _printJob                         { };
    Shepherd*    _shepherd                         { };
    bool         _isPrinterOnline                  { };
    bool         _isFirstOnlineTaskDone            { };
    int          _maxLayerImageWidth               { -1 };

    QPalette     _stopButtonEnabledPalette         { };
    QPalette     _stopButtonDisabledPalette        { };

    std::function<void( )> _initialShowEventFunc;

    void _initialShowEvent( );

signals:

    void stopButtonClicked( );
    void printComplete( );

public slots:

    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );
    void setStopButtonEnabled( bool value );

    void printer_online( );
    void printer_offline( );

    void printManager_printStarting( );
    void printManager_startingLayer( int const layer );
    void printManager_lampStatusChange( bool const on );
    void printManager_printComplete( bool const success );

    void disableSteppers_sendComplete( bool const success );
    void setFanSpeed_sendComplete( bool const success );

protected slots:

private slots:

    void stopButton_clicked( bool );

};

#endif // __STATUSTAB_H__
