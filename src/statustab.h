#ifndef __STATUSTAB_H__
#define __STATUSTAB_H__

#include <QtCore>
#include <QtWidgets>
#include "tabbase.h"

class StatusTab: public InitialShowEventMixin<StatusTab, TabBase> {

    Q_OBJECT

public:

    StatusTab( QWidget* parent = nullptr );
    virtual ~StatusTab( ) override;

    bool isStopButtonEnabled( ) {
        return _stopButton->isEnabled( );
    }

    virtual TabIndex tabIndex( ) const override { return TabIndex::Status; }

protected:

    virtual void _connectPrintManager( )                override;
    virtual void _connectShepherd( )                    override;
    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    QLabel*             _currentLayerDisplay        { new QLabel      };
    QLabel*             _elapsedTimeDisplay         { new QLabel      };
    QLabel*             _estimatedTimeLeftDisplay   { new QLabel      };
    QLabel*             _percentageCompleteDisplay  { new QLabel      };
    QLabel*             _printerStateDisplay        { new QLabel      };
    QLabel*             _temperatureDisplay         { new QLabel      };
    QLabel*             _projectorLampStateDisplay  { new QLabel      };

    QPixmap*            _warningHotImage            { };
    QLabel*             _warningHotLabel            { new QLabel      };

    QPixmap*            _warningUvImage             { };
    QLabel*             _warningUvLabel             { new QLabel      };

    QPushButton*        _pauseButton                { new QPushButton };
    QPushButton*        _stopButton                 { new QPushButton };
    QPushButton*        _reprintButton              { new QPushButton };

    QWidget*            _leftColumn                 { new QWidget     };


    QLabel*             _currentLayerImage          { new QLabel      };
    QLabel*             _modelFileNameLabel         { new QLabel      };
    QLabel*             _imageFileNameLabel         { new QLabel      };
    QVBoxLayout*        _currentLayerLayout         {                 };
    QGroupBox*          _currentLayerGroup          { new QGroupBox   };

    QLabel*             _dispensePrintSolutionLabel { new QLabel      };
    QPushButton*        _startThePrintButton        { new QPushButton };
    QGroupBox*          _dispensePrintSolutionGroup { new QGroupBox   };

    QWidget*            _rightColumn                { new QWidget     };

    QTimer*             _updatePrintTimeInfo        { };
    QTimer*             _printerOnlineTimer         { };

    QFont               _boldFont;
    QFont               _italicFont;

    bool                _isFirstOnlineTaskDone      { false };
    bool                _isPrinterOnline            { false };
    bool                _isPrinterAvailable         { true  };
    bool                _isPrinterPrepared          { false };
    bool                _isModelRendered            { false };
    bool                _isPaused                   { false };

    double              _printJobStartTime          { };
    double              _currentLayerStartTime      { };
    double              _previousLayerStartTime     { };
    double              _estimatedPrintJobTime      { };
    double              _totalPausedTime            { };
    double              _currentPauseStartTime      { };

    std::vector<double> _layerElapsedTimes          { };


    void _updateReprintButtonState( );

signals:

    void printRequested( );

public slots:

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void setModelRendered( bool const value );
    void setPrinterPrepared( bool const value );
    void setPrinterAvailable( bool const value );

protected slots:

private slots:

    void printer_online( );
    void printer_offline( );

    void printManager_printStarting( );
    void printManager_printComplete( bool const success );
    void printManager_printAborted( );
    void printManager_printPausable( bool const pausable );
    void printManager_printPaused( );
    void printManager_printResumed( );
    void printManager_startingLayer( int const layer );
    void printManager_lampStatusChange( bool const on );
    void printManager_requestDispensePrintSolution( );

    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void initializationCommands_sendComplete( bool const success );

    void updatePrintTimeInfo_timeout( );
    void printerOnlineTimer_timeout( );

    void pauseButton_clicked( bool );
    void stopButton_clicked( bool );
    void reprintButton_clicked( bool );
    void startThePrintButton_clicked( bool );

};

#endif // __STATUSTAB_H__
