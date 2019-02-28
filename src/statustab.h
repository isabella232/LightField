#ifndef __STATUSTAB_H__
#define __STATUSTAB_H__

class PrintJob;
class PrintManager;
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

    QLabel*       printerStateLabel          { new QLabel      };
    QLabel*       printerStateDisplay        { new QLabel      };
    QHBoxLayout*  printerStateLayout         { new QHBoxLayout };
    QLabel*       projectorLampStateLabel    { new QLabel      };
    QLabel*       projectorLampStateDisplay  { new QLabel      };
    QHBoxLayout*  projectorLampStateLayout   { new QHBoxLayout };
    QLabel*       jobStateLabel              { new QLabel      };
    QLabel*       jobStateDisplay            { new QLabel      };
    QHBoxLayout*  jobStateLayout             { new QHBoxLayout };
    QLabel*       currentLayerLabel          { new QLabel      };
    QLabel*       currentLayerDisplay        { new QLabel      };
    QHBoxLayout*  currentLayerLayout         { new QHBoxLayout };
    QLabel*       elapsedTimeLabel           { new QLabel      };
    QLabel*       elapsedTimeDisplay         { new QLabel      };
    QHBoxLayout*  elapsedTimeLayout          { new QHBoxLayout };
    QLabel*       estimatedTimeLeftLabel     { new QLabel      };
    QLabel*       estimatedTimeLeftDisplay   { new QLabel      };
    QHBoxLayout*  estimatedTimeLeftLayout    { new QHBoxLayout };
    QLabel*       percentageCompleteLabel    { new QLabel      };
    QLabel*       percentageCompleteDisplay  { new QLabel      };
    QHBoxLayout*  percentageCompleteLayout   { new QHBoxLayout };
    QWidget*      progressControlsContainer  { new QWidget     };
    QVBoxLayout*  progressControlsLayout     { new QVBoxLayout };
    QLabel*       currentLayerImage          { new QLabel      };
    QGroupBox*    currentLayerImageGroup     { new QGroupBox   };
    QPushButton*  stopButton                 { new QPushButton };
    QGridLayout*  _layout                    { new QGridLayout };
    int           _maxLayerImageWidth        { -1 };
    QTimer*       _updatePrintTimeInfo       { };
    PrintJob*     _printJob                  { };
    PrintManager* _printManager              { };
    Shepherd*     _shepherd                  { };
    bool          _isPrinterOnline           { };
    bool          _isFirstOnlineTaskDone     { };
    double        _printJobStartTime         { };
    QPalette      _stopButtonEnabledPalette  { };
    QPalette      _stopButtonDisabledPalette { };

    std::function<void( )> _initialShowEventFunc;

    void _initialShowEvent( );

signals:

    void stopButtonClicked( );
    void printComplete( );

public slots:

    void setPrintJob( PrintJob* printJob );
    void setPrintManager( PrintManager* printManager );
    void setShepherd( Shepherd* shepherd );
    void setStopButtonEnabled( bool value );

    void printer_online( );
    void printer_offline( );

    void printManager_printStarting( );
    void printManager_startingLayer( int const layer );
    void printManager_lampStatusChange( bool const on );
    void printManager_printComplete( bool const success );
    void printManager_printAborted( );

    void disableSteppers_sendComplete( bool const success );
    void setFanSpeed_sendComplete( bool const success );

    void updatePrintTimeInfo_timeout( );

protected slots:

private slots:

    void stopButton_clicked( bool );

};

#endif // __STATUSTAB_H__
