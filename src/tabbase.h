#ifndef __TABBASE_H__
#define __TABBASE_H__

class PrintJob;
class Shepherd;

class TabBase: public QWidget {

    Q_OBJECT

public:

    TabBase( QWidget* parent = nullptr );
    virtual ~TabBase( ) override;

    PrintJob* printJob( ) const { return _printJob; }
    Shepherd* shepherd( ) const { return _shepherd; }

protected:

    Shepherd* _shepherd { };
    PrintJob* _printJob { };

    virtual void _disconnectShepherd( );
    virtual void _connectShepherd( );

private:

signals:

public slots:

    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );

protected slots:

private slots:

};

#endif // !__TABBASE_H__
