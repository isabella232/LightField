#ifndef __STDIOLOGGER_H__
#define __STDIOLOGGER_H__

class StdioLogger: public QObject {

    Q_OBJECT;

public:

    StdioLogger( char const* name, QObject* parent = nullptr );
    virtual ~StdioLogger( ) override;

    void clear( );
    void flush( );

protected:

private:

    char*   _prefix   { };
    QString _lfPrefix;

    QString _buffer;

signals:
    ;

public slots:
    ;

    void read( QString const& data );

protected slots:
    ;

private slots:
    ;

};

#endif // !__STDIOLOGGER_H__
