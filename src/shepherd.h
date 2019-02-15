#ifndef __SHEPHERD_H__
#define __SHEPHERD_H__

enum class PendingCommand {
    none,
    move,
    moveTo,
    home,
};

class Shepherd: public QObject {

    Q_OBJECT

public:

    Shepherd( QObject* parent );
    virtual ~Shepherd( ) override;

    void start( );

    void doMove( float arg );
    void doMoveTo( float arg );
    void doHome( );
    void doSend( char const* arg );
    void doTerminate( );

protected:

private:

    QProcess*      _process;
    QString        _buffer;
    PendingCommand _pendingCommand { PendingCommand::none };
    int            _okCount { };

    QStringList splitLine( QString const& line );
    void        handleFromPrinter( QString const& input );
    void        handleInput( QString const& input );

signals:

    void shepherd_started( );
    void shepherd_finished( int exitCode, QProcess::ExitStatus exitStatus );
    void shepherd_processError( QProcess::ProcessError error );

    void printer_online( );
    void printer_offline( );
    void printer_output( QString const& output );

    void action_moveComplete( bool successful );
    void action_moveToComplete( bool successful );
    void action_homeComplete( bool successful );

public slots:

protected slots:

private slots:

    void processErrorOccurred( QProcess::ProcessError error );
    void processStarted( );
    void processReadyReadStandardError( );
    void processReadyReadStandardOutput( );
    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );

};

char const* ToString( PendingCommand value );

#endif // __SHEPHERD_H__
