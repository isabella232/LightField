#ifndef __PRINT_MANAGER_H__
#define __PRINT_MANAGER_H__

class PngDisplayer;
class PrintJob;
class ProcessRunner;
class Shepherd;

class PrintManager: public QObject {

    Q_OBJECT

public:

    PrintManager( Shepherd* shepherd, QObject* parent = 0 );
    virtual ~PrintManager( ) override;

    void print( PrintJob* printJob );

protected:

private:

    bool           _lampOn               { };
    Shepherd*      _shepherd             { };
    PrintJob*      _printJob             { };
    PngDisplayer*  _pngDisplayer         { };
    ProcessRunner* _setPowerProcess      { };

    int            _currentLayer         { };

    QTimer*        _preProjectionTimer   { };
    QTimer*        _layerProjectionTimer { };
    QTimer*        _preLiftTimer         { };

    void _cleanUp( );

    void _startNextLayer( );

signals:

    void printStarting( );
    void printComplete( bool const success );
    void printAborted( );
    void startingLayer( int const layer );
    void lampStatusChange( bool const on );

public slots:

    void terminate( );
    void abort( );

protected slots:

private slots:

    void step1_LiftUpComplete( bool const success );

    void step2_LiftDownComplete( bool const success );

    void step3_preProjectionTimerExpired( );

    void step4_setPowerCompleted( );
    void step4_setPowerFailed( QProcess::ProcessError const error );

    void step5_layerProjectionTimerExpired( );

    void step6_setPowerCompleted( );
    void step6_setPowerFailed( QProcess::ProcessError const error );

    void step7_preLiftTimerExpired( );

    void step8_LiftUpComplete( bool const success );

    void abort_LiftUpComplete( bool const success );

};

#endif // __PRINT_MANAGER_H__
