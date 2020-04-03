#ifndef __PRINT_MANAGER_H__
#define __PRINT_MANAGER_H__

class PngDisplayer;
class PrintJob;
class ProcessRunner;
class Shepherd;
class OrderManifestManager;

enum class PrintResult {
    Abort   = -2,
    Failure = -1,
    None    =  0,
    Success =  1,
};

enum class PrintStep {
    none,
    A1, A2, A3, A4, A5,
    B1, B2, B3, B4, B5, B6, B7,
    C1, C2,
    D1, D2, D3, D4,
};

class PrintManager: public QObject {

    Q_OBJECT

public:

    PrintManager( Shepherd* shepherd, OrderManifestManager* manifestMgr, QObject* parent = 0 );
    virtual ~PrintManager( ) override;

    int currentLayer( ) const { return _currentLayer; }

    QString& currentLayerImage();

protected:

private:

    Shepherd*      _shepherd                 { };
    PrintJob*      _printJob                 { };
    PngDisplayer*  _pngDisplayer             { };
    ProcessRunner* _setProjectorPowerProcess { };
    PrintResult    _printResult              { };

    bool           _lampOn                   { };
    PrintStep      _step                     { };
    PrintStep      _pausedStep               { };
    int            _currentLayer             { };
    bool           _paused                   { false };
    double         _position                 { };
    double         _pausedPosition           { };
    double         _threshold                { PrinterHighSpeedThresholdZ };

    QTimer*        _preProjectionTimer       { };
    QTimer*        _layerProjectionTimer     { };
    QTimer*        _preLiftTimer             { };

    OrderManifestManager* _manifestMgr;

    QTimer* _makeAndStartTimer( int const duration, void ( PrintManager::*func )( ) );
    void    _stopAndCleanUpTimer( QTimer*& timer );
    void    _pausePrinting( );
    void    _cleanUp( );

signals:

    void requestDispensePrintSolution( );

    void printStarting( );
    void printComplete( bool const success );
    void printAborted( );
    void printPausable( bool const pausable );
    void printPaused( );
    void printResumed( );
    void startingLayer( int const layer );
    void lampStatusChange( bool const on );

public slots:

    void setPngDisplayer( PngDisplayer* pngDisplayer );

    void print( PrintJob* printJob );
    void pause( );
    void resume( );
    void terminate( );
    void abort( );

    void printSolutionDispensed( );

    void printer_positionReport( double px, int cx );

protected slots:

private slots:

    void stepA1_start( );
    void stepA1_completed( bool const success );

    void stepA2_start( );
    void stepA2_completed( );

    void stepA3_start( );
    void stepA3_completed( bool const success );

    void stepA4_start( );
    void stepA4_completed( bool const success );

    void stepA5_start( );
    void stepA5_completed( );

    void stepB1_start( );
    void stepB1_completed( );
    void stepB1_failed( int const exitCode, QProcess::ProcessError const error );

    void stepB2_start( );
    void stepB2_completed( );

    void stepB3_start( );
    void stepB3_completed( );
    void stepB3_failed( int const exitCode, QProcess::ProcessError const error );

    void stepB4_start( );
    void stepB4_completed( );

    void stepB5_start( );
    void stepB5_completed( bool const success );

    void stepB6_start( );
    void stepB6_completed( bool const success );

    void stepB7_start( );
    void stepB7_completed( );


    void stepC1_start( );
    void stepC1_completed( bool const success );

    void stepC2_start( );
    void stepC2_completed( bool const success );


    void stepD1_start( );
    void stepD1_completed( bool const success );

    void stepD2_start( );
    void stepD2_completed( bool const success );

    void stepD3_start( );
    void stepD3_completed( bool const success );

    void stepD4_start( );
    void stepD4_completed( bool const success );

};

#endif // __PRINT_MANAGER_H__
