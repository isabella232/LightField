#ifndef __SHEPHERD_H__
#define __SHEPHERD_H__

#include <QProcess>

class Window;

class Shepherd: public QObject {

    Q_OBJECT

    Shepherd( Shepherd const& ) = delete;
    Shepherd( Shepherd&& ) = delete;
    Shepherd& operator=( Shepherd const& ) = delete;
    Shepherd& operator=( Shepherd&& ) = delete;

public:

    Shepherd( QObject* parent );
    ~Shepherd( );

    void doMove( float arg );
    void doMoveTo( float arg );
    void doHome( );
    void doLift( float arg1, float arg2 );
    void doAskTemp( );
    void doSend( char const* arg );

public slots:

    void processStarted( );
    void processErrorOccurred( QProcess::ProcessError );

private:

    QProcess* _process;

};

#endif // __SHEPHERD_H__
