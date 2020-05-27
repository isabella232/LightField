#ifndef __PRINT_MANAGER_H__
#define __PRINT_MANAGER_H__

#include <QtCore>
#include "constants.h"

class MovementInfo;
class MovementSequencer;
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
    A1, A2, A3,
    B1, B2, B2a, B3,
        B4a1, B4a2,
        B4b1, B4b2,
    C1, C2, C2a, C3,
        C4a1, C4a2,
        C4b1, C4b2,
    D1,
    E1, E2,
};

class PrintManager: public QObject {

    Q_OBJECT

public:

    PrintManager( Shepherd* shepherd, QObject* parent = 0 );
    virtual ~PrintManager( ) override;

    int currentLayer( ) const { return _currentLayer; }

    QString& currentLayerImage();

protected:

private:

    Shepherd*           _shepherd                 { };
    MovementSequencer*  _movementSequencer        { };
    QSharedPointer<PrintJob> _printJob;
    PngDisplayer*       _pngDisplayer             { };
    ProcessRunner*      _setProjectorPowerProcess { };
    PrintResult         _printResult              { };

    bool                _lampOn                   { };
    bool                _duringTiledLayer         {false};
    PrintStep           _step                     { };
    PrintStep           _pausedStep               { };
    int                 _currentLayer             { };
    int                 _currentBaseLayer         { };
    int                 _elementsOnLayer          { };
    bool                _isTiled                  { false };
    bool                _paused                   { false };
    double              _position                 { };
    double              _pausedPosition           { };
    double              _threshold                { PrinterHighSpeedThresholdZ };

    QTimer*             _preProjectionTimer       { };
    QTimer*             _layerExposureTimer       { };
    QTimer*             _preLiftTimer             { };

    QList<MovementInfo> _stepA1_movements;
    QList<MovementInfo> _stepA3_movements;
    QList<MovementInfo> _stepB4a2_movements;
    QList<MovementInfo> _stepB4b2_movements;
    QList<MovementInfo> _stepC4a2_movements;
    QList<MovementInfo> _stepC4b2_movements;

    QTimer* _makeAndStartTimer( int const duration, void ( PrintManager::*func )( ) );
    void    _stopAndCleanUpTimer( QTimer*& timer );
    void    _pausePrinting( );
    void    _cleanUp( );
    bool    _hasLayerMoreElements();

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
    ;

    void setPngDisplayer( PngDisplayer* pngDisplayer );

    void print(QSharedPointer<PrintJob> printJob);
    void pause( );
    void resume( );
    void terminate( );
    void abort( );

    void printSolutionDispensed( );

    void printer_positionReport( double px, int cx );

protected slots:
    ;

private slots:
    ;

    void stepA1_start( );
    void stepA1_completed( bool const success );

    void stepA2_start( );
    void stepA2_completed( );

    void stepA3_start( );
    void stepA3_completed( bool const success );


    void stepB1_start( );
    void stepB1_completed( );
    void stepB1_failed( int const exitCode, QProcess::ProcessError const error );

    void stepB2_start( );
    void stepB2_completed( );

    void stepB2a_start( );

    void stepB3_start( );
    void stepB3_completed( );
    void stepB3_failed( int const exitCode, QProcess::ProcessError const error );

    void stepB4a1_start( );
    void stepB4a1_completed( );

    void stepB4a2_start( );
    void stepB4a2_completed( bool const success );

    void stepB4b1_start( );
    void stepB4b1_completed( );

    void stepB4b2_start( );
    void stepB4b2_completed( bool const success );

    void stepC1_start( );
    void stepC1_completed( );
    void stepC1_failed( int const exitCode, QProcess::ProcessError const error );

    void stepC2_start( );
    void stepC2_completed( );

    void stepC2a_start( );

    void stepC3_start( );
    void stepC3_completed( );
    void stepC3_failed( int const exitCode, QProcess::ProcessError const error );

    void stepC4a1_start( );
    void stepC4a1_completed( );

    void stepC4a2_start( );
    void stepC4a2_completed( bool const success );

    void stepC4b1_start( );
    void stepC4b1_completed( );

    void stepC4b2_start( );
    void stepC4b2_completed( bool const success );


    void stepD1_start( );
    void stepD1_completed( bool const success );


    void stepE1_start( );
    void stepE1_completed( bool const success );

    void stepE2_start( );
    void stepE2_completed( bool const success );

};

#endif // __PRINT_MANAGER_H__
