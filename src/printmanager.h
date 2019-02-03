#ifndef __PRINT_MANAGER_H__
#define __PRINT_MANAGER_H__

#include <QObject>

#include "printjob.h"

class Shepherd;
class QProcess;

class PrintManager: public QObject {

    Q_OBJECT

public:

    PrintManager( Shepherd* shepherd, QObject* parent = 0 );
    virtual ~PrintManager( ) override;

    void print( PrintJob* printJob );

protected:

private:

    PrintJob* _printJob     { };
    Shepherd* _shepherd     { };
    QProcess* _fehProcess   { };
    int       _currentLayer { };

signals:

    void printComplete( bool success );
    void startingLayer( int layer );

public slots:

protected slots:

private slots:

    void processErrorOccurred( QProcess::ProcessError error );
    void processStarted( );
    void processStateChanged( QProcess::ProcessState newState );
    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void initialHomeComplete( bool success );

};

#endif // __PRINT_MANAGER_H__