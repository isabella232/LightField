#ifndef __WELCOMETAB_H__
#define __WELCOMETAB_H__

class PrintJob;
class Shepherd;

class WelcomeTab: public QWidget {

    Q_OBJECT

public:

    WelcomeTab( QWidget* parent = nullptr );
    virtual ~WelcomeTab( ) override;
    
    Shepherd* shepherd( ) const { return _shepherd; }

protected:

private:

    PrintJob*    _printJob     { };
    Shepherd*    _shepherd     { };

    QLabel*      _logoLabel    { new QLabel      };
    QLabel*      _versionLabel { new QLabel      };

    QVBoxLayout* _layout       { new QVBoxLayout };

signals:

public slots:

    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );

protected slots:

private slots:

};

#endif // __WELCOMETAB_H__
