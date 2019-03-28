#ifndef __TABBASE_H__
#define __TABBASE_H__

#include "initialshoweventmixin.h"
#include "tabindex.h"
#include "uistate.h"

class PrintJob;
class PrintManager;
class Shepherd;

class TabBase: public QWidget {

    Q_OBJECT

public:

    TabBase( QWidget* parent = nullptr );
    virtual ~TabBase( ) override;

    PrintJob*        printJob( )     const { return _printJob;     }
    PrintManager*    printManager( ) const { return _printManager; }
    Shepherd*        shepherd( )     const { return _shepherd;     }
    UiState          uiState( )      const { return _uiState;      }

    virtual TabIndex tabIndex( )     const = 0;

protected:

    PrintJob*     _printJob     { };
    PrintManager* _printManager { };
    Shepherd*     _shepherd     { };
    UiState       _uiState      { };

    virtual void _disconnectPrintJob( );
    virtual void _connectPrintJob( );

    virtual void _disconnectPrintManager( );
    virtual void _connectPrintManager( );

    virtual void _disconnectShepherd( );
    virtual void _connectShepherd( );

private:

signals:

    void uiStateChanged( TabIndex const sender, UiState const state );

public slots:

    virtual void setPrintJob( PrintJob* printJob );
    virtual void setPrintManager( PrintManager* printManager );
    virtual void setShepherd( Shepherd* shepherd );
    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) = 0;

protected slots:

private slots:

};

#endif // !__TABBASE_H__
