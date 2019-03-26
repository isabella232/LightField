#ifndef __TABBASE_H__
#define __TABBASE_H__

#include "uistate.h"

using namespace std::placeholders;

class PrintJob;
class PrintManager;
class Shepherd;

class TabBase: public QWidget {

    Q_OBJECT

public:

    TabBase( QWidget* parent = nullptr );
    virtual ~TabBase( ) override;

    UiState       uiState( )      const { return _uiState;      }
    PrintJob*     printJob( )     const { return _printJob;     }
    PrintManager* printManager( ) const { return _printManager; }
    Shepherd*     shepherd( )     const { return _shepherd;     }

protected:

    PrintJob*     _printJob     { };
    PrintManager* _printManager { };
    Shepherd*     _shepherd     { };
    UiState       _uiState      { };

    std::function<void( QShowEvent* event )> _initialShowEventFunc;

    virtual void showEvent( QShowEvent* event ) override;
    virtual void _initialShowEvent( QShowEvent* event );

    virtual void _disconnectPrintJob( );
    virtual void _connectPrintJob( );

    virtual void _disconnectPrintManager( );
    virtual void _connectPrintManager( );

    virtual void _disconnectShepherd( );
    virtual void _connectShepherd( );

private:

signals:

    void uiStateChanged( UiState const state );

public slots:

    virtual void setUiState( UiState const state );

    virtual void setPrintJob( PrintJob* printJob );
    virtual void setPrintManager( PrintManager* printManager );
    virtual void setShepherd( Shepherd* shepherd );

protected slots:

private slots:

};

#endif // !__TABBASE_H__
