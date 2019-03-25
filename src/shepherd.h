#ifndef __SHEPHERD_H__
#define __SHEPHERD_H__

enum class PendingCommand {
    none,
    moveRelative,
    moveAbsolute,
    home,
    send,
};

class Shepherd: public QObject {

    Q_OBJECT

public:

    Shepherd( QObject* parent );
    virtual ~Shepherd( ) override;

    void start( );

    void doMoveRelative( float const relativeDistance );
    void doMoveAbsolute( float const absolutePosition );
    void doHome( );
    void doSend( QString cmd );
    void doSend( QStringList cmds );
    void doTerminate( );

protected:

private:

    QProcess*      _process;
    QString        _buffer;
    PendingCommand _pendingCommand        { PendingCommand::none };
    int            _okCount               { };
    int            _expectedOkCount       { };
    bool           _isTerminationExpected { };

    bool           getReady( char const* functionName, PendingCommand const pendingCommand, int const expectedOkCount = 0 );
    QStringList    splitLine( QString const& line );
    void           handleFromPrinter( QString const& input );
    void           handleCommandFail( QStringList const& input );
#if defined _DEBUG
    void           handleCommandFailAlternate( QStringList const& input );
#endif // defined _DEBUG
    void           handleInput( QString const& input );

    void           doSendOne( QString& cmd );

signals:

    void shepherd_started( );
    void shepherd_startFailed( );
    void shepherd_terminated( bool const expected, bool const cleanExit );

    void printer_online( );
    void printer_offline( );
    void printer_output( QString const& output );
    void printer_positionReport( double const px, double const py, double const pz, double const pe, double const cx, double const cy, double const cz );
    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void action_moveRelativeComplete( bool const successful );
    void action_moveAbsoluteComplete( bool const successful );
    void action_homeComplete( bool const successful );
    void action_sendComplete( bool const successful );

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
