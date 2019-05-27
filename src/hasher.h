#ifndef __HASHER_H__
#define __HASHER_H__

class Hasher: public QObject {

    QCryptographicHash::Algorithm const _DefaultAlgorithm = QCryptographicHash::Md5;

    Q_OBJECT

public:

    Hasher( QObject* parent = nullptr ): QObject( parent ) {
        /*empty*/
    }

    virtual ~Hasher( ) override {
        /*empty*/
    }

    void hash( QString const fileName ) {
        _thread = QThread::create( std::bind( &Hasher::_hash, this, fileName, QCryptographicHash::Md5 ) );
        QObject::connect( _thread, &QThread::finished, _thread, &QThread::deleteLater );
        _thread->start( );
    }

    void hash( QString const fileName, QCryptographicHash::Algorithm const algorithm ) {
        _thread = QThread::create( std::bind( &Hasher::_hash, this, fileName, algorithm ) );
        QObject::connect( _thread, &QThread::finished, _thread, &QThread::deleteLater );
        _thread->start( );
    }

    void checkHashes( QMap<QString, QString> const fileNames, QCryptographicHash::Algorithm const algorithm ) {
        _thread = QThread::create( std::bind( &Hasher::_checkHashes, this, fileNames, algorithm ) );
        QObject::connect( _thread, &QThread::finished, _thread, &QThread::deleteLater );
        _thread->start( );
    }

protected:

private:

    QThread* _thread;

    void _hash( QString const fileName, QCryptographicHash::Algorithm const algorithm );
    void _checkHashes( QMap<QString, QString> const fileNames, QCryptographicHash::Algorithm const algorithm );

signals:

    void resultReady( QString const hash );
    void hashCheckResult( bool const result );

public slots:

protected slots:

private slots:

};

#endif // __HASHER_H__
