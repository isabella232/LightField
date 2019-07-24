#ifndef __COMMANDREADER_H__
#define __COMMANDREADER_H__

class CommandReader: public QObject {

    Q_OBJECT

public:

    CommandReader( QObject* parent = nullptr );
    virtual ~CommandReader( ) override;

protected:

private:

    QThread* _thread;

signals:
    ;

    void _commandReceived( QString const );

    void commandReceived( QStringList const& );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void thread_commandReceived( QString const command );

};

#endif // !__COMMANDREADER_H__
