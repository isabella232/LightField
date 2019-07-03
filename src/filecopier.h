#ifndef __FILE_COPIER__
#define __FILE_COPIER__

#include "initialshoweventmixin.h"

class FileCopier: public InitialShowEventMixin<FileCopier, QMainWindow> {

    Q_OBJECT

public:

    FileCopier( QWidget* parent = nullptr );
    virtual ~FileCopier( ) override;

    void setFileList( QVector<QPair<QString, QString>> const& fileList );

    void startCopy( );
    void abortCopy( );

protected:

    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    QVector<QPair<QString, QString>> _fileList;

signals:
    ;

    void progress( int const index, quint64 const fileBytesCopied, quint64 const fileBytesTotal, quint64 const totalBytesCopied, quint64 const totalBytesTotal );
    void error( int const index, QString const& message );
    void finished( );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif // !__FILE_COPIER__
