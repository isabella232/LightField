#ifndef __SIGNALHANDLER_H__
#define __SIGNALHANDLER_H__

class SignalHandler: public QObject {

    Q_OBJECT

public:

    SignalHandler( QObject* parent = nullptr );
    virtual ~SignalHandler( ) override;

    void subscribe( int const signum );
    void subscribe( std::initializer_list<int> signums );

    void unsubscribe( int const signum );
    void unsubscribe( std::initializer_list<int> signums );

protected:

private:

    void _cleanUp( );

    QSocketNotifier* _signalNotifier { };

private /*static*/:

    static void _signalAction( int /*signalNumber*/, siginfo_t* info, void* /*ucontext*/ );

signals:
    ;

    void signalReceived( siginfo_t const& info );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void dispatchSignal( int );

};

#endif // __SIGNALHANDLER_H__
