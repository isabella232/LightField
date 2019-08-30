#ifndef __FILECOPIER_H__
#define __FILECOPIER_H__

using FileNamePair     = QPair<QString, QString>;
using FileNamePairList = QList<FileNamePair>;

class FileCopier: public QObject {

    Q_OBJECT

public:

    FileCopier( QObject* parent = nullptr );
    virtual ~FileCopier( ) override;

    void copy( FileNamePairList const& fileList );
    void abort( );

    bool isAborted( ) const {
        return _abortRequested;
    }

    bool isStarted( ) const {
        return _started;
    }

    bool isCopying( ) const {
        return isStarted( ) && !isAborted( );
    }

protected:

private:

    FileNamePairList _fileList;
    QThread*         _thread;

    std::atomic_bool _started        { };
    std::atomic_bool _abortRequested { };

    void _copy( );

signals:
    ;

    void fileStarted( int const index, qint64 const totalSize );
    void fileProgress( int const index, qint64 const bytesCopied );
    void fileFinished( int const index, qint64 const bytesCopied );

    void notify( int const index, QString const message );
    void finished( int const copiedFiles, int const skippedFiles );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif // __FILECOPIER_H__
