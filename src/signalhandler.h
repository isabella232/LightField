#ifndef __SIGNALHANDLER_H__
#define __SIGNALHANDLER_H__

#include <QSocketNotifier>

class SignalHandler: public QObject {

    Q_OBJECT

public:

    SignalHandler( QObject* parent = nullptr );
    virtual ~SignalHandler( ) override;

protected:

private:

    static void signalHandler( int signum );

    void cleanUp( );

    QSocketNotifier* signalNotifier { nullptr };

signals:

    void quit( int signalNumber );

public slots:

protected slots:

private slots:

    void dispatchSignal( int );

};

extern SignalHandler* g_signalHandler;

#endif // __SIGNALHANDLER_H__
