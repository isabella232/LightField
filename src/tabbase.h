#ifndef __TABBASE_H__
#define __TABBASE_H__

class PrintJob;
class PrintManager;
class Shepherd;

class TabBase: public QWidget {

    Q_OBJECT

public:

    TabBase( QWidget* parent = nullptr );
    virtual ~TabBase( ) override;

    PrintJob*     printJob( )     const { return _printJob;     }
    PrintManager* printManager( ) const { return _printManager; }
    Shepherd*     shepherd( )     const { return _shepherd;     }

protected:

    Shepherd*     _shepherd     { };
    PrintJob*     _printJob     { };
    PrintManager* _printManager { };

    virtual void _disconnectShepherd( );
    virtual void _connectShepherd( );

    virtual void _disconnectPrintManager( );
    virtual void _connectPrintManager( );

private:

signals:

public slots:

    void setPrintJob( PrintJob* printJob );
    void setPrintManager( PrintManager* printManager );
    void setShepherd( Shepherd* shepherd );

protected slots:

private slots:

};

#endif // !__TABBASE_H__
