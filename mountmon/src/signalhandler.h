#if ! defined __SIGNALHANDLER_H__
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

    static void signalHandler( int signum );

    void cleanUp( );

    QSocketNotifier* signalNotifier { nullptr };

signals:

    void signalReceived( int const signalNumber );

public slots:

protected slots:

private slots:

    void dispatchSignal( int );

};

extern SignalHandler* g_signalHandler;

#endif // __SIGNALHANDLER_H__
