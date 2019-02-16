#ifndef __STATUSTAB_H__
#define __STATUSTAB_H__

class PrintJob;

class StatusTab: public QWidget {

    Q_OBJECT

public:

    StatusTab( QWidget* parent = nullptr );
    virtual ~StatusTab( ) override;

    bool isStopButtonEnabled( ) {
        return stopButton->isEnabled( );
    }

protected:

private:

    QLabel*      printerStateLabel          { new QLabel      };
    QLabel*      printerStateDisplay        { new QLabel      };
    QLabel*      projectorLampStateLabel    { new QLabel      };
    QLabel*      projectorLampStateDisplay  { new QLabel      };
    QLabel*      jobStateLabel              { new QLabel      };
    QLabel*      jobStateDisplay            { new QLabel      };
    QLabel*      currentLayerLabel          { new QLabel      };
    QLabel*      currentLayerDisplay        { new QLabel      };
    QVBoxLayout* progressControlsLayout     { new QVBoxLayout };
    QWidget*     progressControlsContainer  { new QWidget     };
    QLabel*      currentLayerImageLabel     { new QLabel      };
    QLabel*      currentLayerImageDisplay   { new QLabel      };
    QVBoxLayout* currentLayerImageLayout    { new QVBoxLayout };
    QWidget*     currentLayerImageContainer { new QWidget     };
    QPushButton* stopButton                 { new QPushButton };
    QGridLayout* _layout                    { new QGridLayout };
    PrintJob*    _printJob                  { };
    bool         _isPrinterOnline           { };

    QPalette     _stopButtonEnabledPalette  { };
    QPalette     _stopButtonDisabledPalette { };

signals:

    void stopButtonClicked( );
    void printComplete( );

public slots:

    void setPrintJob( PrintJob* printJob );
    void setStopButtonEnabled( bool value );

    void printer_online( );
    void printer_offline( );

    void printManager_printStarting( );
    void printManager_startingLayer( int const layer );
    void printManager_lampStatusChange( bool const on );
    void printManager_printComplete( bool const success );

protected slots:

private slots:

    void stopButton_clicked( bool checked );

};

#endif // __STATUSTAB_H__
