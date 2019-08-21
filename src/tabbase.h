#ifndef __TABBASE_H__
#define __TABBASE_H__

#include "initialshoweventmixin.h"

class PrintJob;
class PrintManager;
class Shepherd;

class TabBase: public QWidget {

    Q_OBJECT;
    Q_PROPERTY( PrintJob*     printJob     READ printJob     WRITE setPrintJob     );
    Q_PROPERTY( PrintManager* printManager READ printManager WRITE setPrintManager );
    Q_PROPERTY( Shepherd*     shepherd     READ shepherd     WRITE setShepherd     );
    Q_PROPERTY( TabIndex      tabIndex     READ tabIndex                           );
    Q_PROPERTY( UiState       uiState      READ uiState                            );

public:

    enum class TabIndex {
        File,
        Prepare,
        Print,
        Status,
        Advanced,
        System,
    };
    Q_ENUM( TabIndex );

    enum class UiState {
        SelectStarted,
        SelectCompleted,
        SliceStarted,
        SliceCompleted,
        PrintStarted,
        PrintCompleted,
    };
    Q_ENUM( UiState );

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
    void iconChanged( TabIndex const sender, QIcon const& icon );

public slots:

    virtual void setPrintJob( PrintJob* printJob );
    virtual void setPrintManager( PrintManager* printManager );
    virtual void setShepherd( Shepherd* shepherd );
    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) = 0;

protected slots:

private slots:

};

using TabIndex = TabBase::TabIndex;
using UiState  = TabBase::UiState;

inline int operator+( TabIndex const value ) { return static_cast<int>( value ); }
inline int operator+( UiState  const value ) { return static_cast<int>( value ); }

char const* ToString( TabIndex const value );
char const* ToString( UiState  const value );

#endif // !__TABBASE_H__
