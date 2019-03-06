#ifndef __PRINT_MANAGER_H__
#define __PRINT_MANAGER_H__

class PngDisplayer;
class PrintJob;
class ProcessRunner;
class Shepherd;

class PrintManager: public QObject {

    using TimerExpiryFunc = void ( PrintManager::* )( );

    Q_OBJECT

public:

    PrintManager( Shepherd* shepherd, QObject* parent = 0 );
    virtual ~PrintManager( ) override;

    void print( PrintJob* printJob );

    int currentLayer( ) const {
        return _currentLayer;
    }

protected:

private:

    bool           _lampOn               { };
    bool           _aborting             { };
    bool           _success              { };
    Shepherd*      _shepherd             { };
    PrintJob*      _printJob             { };
    PngDisplayer*  _pngDisplayer         { };
    ProcessRunner* _setpowerProcess      { };

    int            _currentLayer         { };

    QTimer*        _preProjectionTimer   { };
    QTimer*        _layerProjectionTimer { };
    QTimer*        _preLiftTimer         { };

    QTimer* _makeAndStartTimer( int const duration, TimerExpiryFunc func );
    void    _stopAndCleanUpTimer( QTimer*& timer );
    void    _cleanUp( );

signals:

    void requestLoadPrintSolution( );
    void printStarting( );
    void printComplete( bool const success );
    void printAborted( );
    void startingLayer( int const layer );
    void lampStatusChange( bool const on );

public slots:

    void printSolutionLoaded( );

    void terminate( );
    void abort( );

protected slots:

private slots:

    void stepA1_start( );
    void stepA1_completed( bool const success );

    void stepA2_start( );
    void stepA2_completed( );

    void stepA3_start( );
    void stepA3_completed( bool const success );

    void stepA4_start( );
    void stepA4_completed( );


    void stepB1_start( );
    void stepB1_completed( );
    void stepB1_failed( QProcess::ProcessError const error );

    void stepB2_start( );
    void stepB2_completed( );

    void stepB3_start( );
    void stepB3_completed( );
    void stepB3_failed( QProcess::ProcessError const error );

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

};

#endif // __PRINT_MANAGER_H__
