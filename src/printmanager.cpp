#include "printmanager.h"
#include <QProcess>

#include "printmanager.h"
#include "shepherd.h"
#include "strings.h"

PrintManager::PrintManager( Shepherd* shepherd, QObject* parent ): QObject( parent ), _shepherd( shepherd ) {
    _fehProcess = new QProcess( this );
    _fehProcess->setProgram( "feh" );
    //_fehProcess->setArguments( QStringList {
    //    // "feh -x --geometry 1280x800 /home/lumen/Volumetric/model-library/makerook_imgs/" + '%03d' % i + ".png &"
    //    "-x",
    //    "--geometry",
    //    "1280x800",
    //    fileName
    //} );
    QObject::connect( _fehProcess, &QProcess::errorOccurred, this, &PrintManager::processErrorOccurred );
    QObject::connect( _fehProcess, &QProcess::started,       this, &PrintManager::processStarted       );
    QObject::connect( _fehProcess, &QProcess::stateChanged,  this, &PrintManager::processStateChanged  );
    QObject::connect( _fehProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::processFinished );
}

PrintManager::~PrintManager( ) {
    /*empty*/
}

void PrintManager::processErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "+ PrintManager::processErrorOccurred: error %s [%d]\n", ToString( error ), error );
}

void PrintManager::processStarted( ) {
    fprintf( stderr, "+ PrintManager::processStarted\n" );
}

void PrintManager::processStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "+ PrintManager::processStateChanged: new state %s [%d]\n", ToString( newState ), newState );
}

void PrintManager::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    fprintf( stderr, "+ PrintManager::processFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );
}

//
// For each layer:
//
// 1. Lift up
//   G91
//   G1 X${liftDistance}
// 2. Lift down
//   G91
//   G1 X-${liftDistance-layerHeight}
// 3. Start feh
// 4. Pause before projection
// 5. Start projecting: setpower ${brightness}
// 6. Pause for layer time
// 7. Stop projection: setpower 0
// 8. Stop feh
// 9. Pause before lift
//

void PrintManager::initialHomeComplete( bool success ) {
    if ( !success ) {
        fprintf( stderr, "+ PrintManager::initialHomeComplete: move failed\n" );
        emit printComplete( false );
        return;
    }


}

void PrintManager::print( PrintJob* printJob ) {
    if ( _printJob ) {
        fprintf( stderr, "+ PrintManager::print: Job submitted while we're busy\n" );
        return;
    }
    _printJob = printJob;
    QObject::connect( _shepherd, &Shepherd::action_homeComplete, this, &PrintManager::initialHomeComplete );
    _shepherd->doHome( );
}
