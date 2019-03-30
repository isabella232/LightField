#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "coordinate.h"
#include "initialshoweventmixin.h"
#include "tabindex.h"
#include "uistate.h"

class ModelSelectionInfo;
class Shepherd;
class PrintManager;
class PrintJob;

class FileTab;
class PrepareTab;
class PrintTab;
class StatusTab;
class AdvancedTab;
class MaintenanceTab;

class Window: public InitialShowEventMixin<Window, QMainWindow> {

    Q_OBJECT

public:

    Window( QWidget* parent = nullptr );
    virtual ~Window( ) override;

protected:

    virtual void closeEvent( QCloseEvent* event ) override;
    virtual void initialShowEvent( QShowEvent* event ) override;

private:

    Shepherd*           _shepherd          { };
    PrintJob*           _printJob          { };
    PrintManager*       _printManager      { };
    UiState             _uiState           { };
    ModelSelectionInfo* _modelSelection    { };

    QTabWidget*         _tabWidget         { new QTabWidget };

    FileTab*            _fileTab;
    PrepareTab*         _prepareTab;
    PrintTab*           _printTab;
    StatusTab*          _statusTab;
    AdvancedTab*        _advancedTab;
    MaintenanceTab*     _maintenanceTab;

    bool                _isPrinterPrepared { };
    bool                _isModelRendered   { };

    void _setPrinterPrepared( bool const value );
    void _setModelRendered( bool const value );

signals:

    void printJobChanged( PrintJob* printJob );
    void printManagerChanged( PrintManager* printManager );
    void shepherdChanged( Shepherd* shepherd );

    void modelRendered( bool const value );
    void printerPrepared( bool const value );

public slots:

    void startPrinting( );

protected slots:

private slots:

    void tab_uiStateChanged( TabIndex const sender, UiState const state );
    void tabs_currentChanged( int index );

    void shepherd_started( );
    void shepherd_startFailed( );
    void shepherd_terminated( bool const expected, bool const cleanExit );

    void printManager_printStarting( );
    void printManager_printComplete( bool const success );
    void printManager_printAborted( );

    void fileTab_modelSelected( ModelSelectionInfo* modelSelection );

    void prepareTab_slicingNeeded( bool const needed );
    void prepareTab_preparePrinterStarted( );
    void prepareTab_preparePrinterComplete( bool const success );

    void signalHandler_signalReceived( int const signalNumber );

};

#endif // __WINDOW_H__
