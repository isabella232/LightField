#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "coordinate.h"
#include "initialshoweventmixin.h"
#include "tabindex.h"
#include "uistate.h"

class ModelSelectionInfo;
class PngDisplayer;
class PrintManager;
class PrintJob;
class Shepherd;
class UpgradeManager;
class UsbMountManager;

class FileTab;
class PrepareTab;
class PrintTab;
class StatusTab;
class AdvancedTab;
class SystemTab;

class Window: public InitialShowEventMixin<Window, QMainWindow> {

    Q_OBJECT

public:

    Window( QWidget* parent = nullptr );
    virtual ~Window( ) override;

protected:

    virtual void closeEvent( QCloseEvent* event )       override;
    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    ModelSelectionInfo* _modelSelection    { };
    PngDisplayer*       _pngDisplayer      { };
    PrintJob*           _printJob          { };
    PrintManager*       _printManager      { };
    Shepherd*           _shepherd          { };
    UiState             _uiState           { };
    UpgradeManager*     _upgradeManager    { };
    UsbMountManager*    _usbMountManager   { };

    QTabWidget*         _tabWidget         { new QTabWidget };
    FileTab*            _fileTab;
    PrepareTab*         _prepareTab;
    PrintTab*           _printTab;
    StatusTab*          _statusTab;
    AdvancedTab*        _advancedTab;
    SystemTab*          _systemTab;

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

protected slots:

private slots:

    void startPrinting( );

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

Window* getMainWindow( );

#endif // __WINDOW_H__
