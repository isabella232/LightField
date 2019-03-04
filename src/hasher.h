#ifndef __HASHER_H__
#define __HASHER_H__

class Hasher: public QObject {

    Q_OBJECT

public:

    Hasher( QObject* parent = nullptr ): QObject( parent ) {
        /*empty*/
    }

    virtual ~Hasher( ) override {
        /*empty*/
    }

    void hash( QString const fileName ) {
        _thread = QThread::create( std::bind( &Hasher::_hash, this, fileName ) );
        QObject::connect( _thread, &QThread::finished, _thread, &QThread::deleteLater );
        _thread->start( );
    }

protected:

private:

    QThread* _thread;

    void _hash( QString const fileName );

signals:

    void resultReady( QString const hash );

public slots:

protected slots:

private slots:

};

#endif // __HASHER_H__
