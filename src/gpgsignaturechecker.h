#ifndef __GPGSIGNATURECHECKER_H__
#define __GPGSIGNATURECHECKER_H__

class ProcessRunner;

class GpgSignatureChecker: public QObject {

    Q_OBJECT

public:

    GpgSignatureChecker( QObject* parent = nullptr );
    virtual ~GpgSignatureChecker( ) override;

    void startCheckDetachedSignature( QString const& dataFileName, QString const& signatureFileName );

    int instanceId( ) const;
    QProcess::ProcessState state( ) const;

protected:

private:

    ProcessRunner* _processRunner { };
    QString        _dataFileName;
    QString        _signatureFileName;
    QString        _result;

signals:

    void signatureCheckComplete( bool const result, QStringList const& results );

public slots:

protected slots:

private slots:

    void gpg_succeeded( );
    void gpg_failed( int const exitCode, QProcess::ProcessError const error );
    void gpg_readyReadStandardOutput( QString const& data );

};

#endif // __GPGSIGNATURECHECKER_H__
