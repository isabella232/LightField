#ifndef __STATUSTAB_H__
#define __STATUSTAB_H__

#include "tabbase.h"

class StatusTab: public TabBase {

    Q_OBJECT

public:

    StatusTab( QWidget* parent = nullptr );
    virtual ~StatusTab( ) override;

    bool isStopButtonEnabled( ) {
        return _stopButton->isEnabled( );
    }

protected:

private:

    QLabel*                _printerStateLabel         { new QLabel      };
    QLabel*                _printerStateDisplay       { new QLabel      };
    QLabel*                _projectorLampStateLabel   { new QLabel      };
    QLabel*                _projectorLampStateDisplay { new QLabel      };
    QLabel*                _jobStateLabel             { new QLabel      };
    QLabel*                _jobStateDisplay           { new QLabel      };
    QLabel*                _currentLayerLabel         { new QLabel      };
    QLabel*                _currentLayerDisplay       { new QLabel      };
    QLabel*                _elapsedTimeLabel          { new QLabel      };
    QLabel*                _elapsedTimeDisplay        { new QLabel      };
    QLabel*                _estimatedTimeLeftLabel    { new QLabel      };
    QLabel*                _estimatedTimeLeftDisplay  { new QLabel      };
    QLabel*                _percentageCompleteLabel   { new QLabel      };
    QLabel*                _percentageCompleteDisplay { new QLabel      };
    QWidget*               _progressControlsContainer { new QWidget     };
    QVBoxLayout*           _progressControlsLayout    { new QVBoxLayout };

    QLabel*                _currentLayerImage         { new QLabel      };
    QVBoxLayout*           _currentLayerLayout        {                 };
    QGroupBox*             _currentLayerGroup         { new QGroupBox   };

    QLabel*                _loadPrintSolutionLabel    { new QLabel      };
    QPushButton*           _printSolutionLoadedButton { new QPushButton };
    QGroupBox*             _loadPrintSolutionGroup    { new QGroupBox   };

    QLabel*                _warningHotLabel           { new QLabel      };
    QLabel*                _warningUvLabel            { new QLabel      };

    QPushButton*           _stopButton                { new QPushButton };

    QGridLayout*           _layout                    { new QGridLayout };
    QTimer*                _updatePrintTimeInfo       { };

    QPixmap*               _warningHotImage           { };
    QPixmap*               _warningUvImage            { };

    QPalette               _stopButtonEnabledPalette  { };
    QPalette               _stopButtonDisabledPalette { };

    bool                   _isPrinterOnline           { };
    bool                   _isFirstOnlineTaskDone     { };

    double                 _printJobStartTime         { };
    double                 _currentLayerStartTime     { };
    double                 _previousLayerStartTime    { };
    double                 _estimatedPrintJobTime     { };
    std::vector<double>    _layerElapsedTimes         { };

    virtual void _initialShowEvent( QShowEvent* showEvent ) override;
    virtual void _connectPrintManager( )                    override;
    virtual void _connectShepherd( )                        override;

signals:

    void stopButtonClicked( );
    void printComplete( );

public slots:

    void setStopButtonEnabled( bool value );

    void printer_online( );
    void printer_offline( );

    void printManager_printStarting( );
    void printManager_startingLayer( int const layer );
    void printManager_lampStatusChange( bool const on );
    void printManager_printComplete( bool const success );
    void printManager_printAborted( );

    void shepherd_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void initializationCommands_sendComplete( bool const success );

    void updatePrintTimeInfo_timeout( );

protected slots:

private slots:

    void stopButton_clicked( bool );
    void printManager_requestLoadPrintSolution( );
    void printSolutionLoadedButton_clicked( bool );

};

#endif // __STATUSTAB_H__
