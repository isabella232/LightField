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

    void _readCommands( );

signals:
    ;

    void commandReceived( QStringList const );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif // !__COMMANDREADER_H__
