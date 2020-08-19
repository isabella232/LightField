#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <csignal>
#include <QtCore>
#include <QtWidgets>
#include "coordinate.h"
#include "tabbase.h"
#include "tilingtab.h"

class ModelSelectionInfo;
class PngDisplayer;
class PrintManager;
class PrintProfileManager;
class FirmwareController;
class SignalHandler;
class UpgradeManager;
class UsbMountManager;

class FileTab;
class PrepareTab;
class PrintTab;
class StatusTab;
class AdvancedTab;
class ProfilesTab;
class SystemTab;

class Window: public QMainWindow {

    Q_OBJECT

public:

    Window( QWidget* parent = nullptr );
    virtual ~Window( ) override;
    void terminate( );

protected:

    virtual void closeEvent( QCloseEvent* event ) override;
    virtual void showEvent( QShowEvent* aShowEvent ) override;

private:

    SignalHandler*       _signalHandler       { };
    ModelSelectionInfo*  _modelSelection      { };
    PngDisplayer*        _pngDisplayer        { };
    PrintManager*        _printManager        { };
    PrintProfileManager* _printProfileManager { };
    FirmwareController*  _firmwareController  { };
    UiState              _uiState             { };
    UpgradeManager*      _upgradeManager      { };
    UsbMountManager*     _usbMountManager     { };

    QTabWidget*          _tabWidget           { new QTabWidget  };
    FileTab*             _fileTab;
    PrepareTab*          _prepareTab;
    TilingTab*           _tilingTab;
    PrintTab*            _printTab;
    StatusTab*           _statusTab;
    AdvancedTab*         _advancedTab;
    ProfilesTab*         _profilesTab;
    SystemTab*           _systemTab;
    QPushButton*         _helpButton          { new QPushButton };

    bool                 _isPrinterPrepared   { };
    bool                 _isModelRendered     { };

    void _setPrinterPrepared( bool const value );
    void _setModelRendered( bool const value );

signals:

    void printManagerChanged(PrintManager* printManager);
    void firmwareControllerChanged(FirmwareController* controller);

    void modelRendered(bool const value);
    void printerPrepared(bool const value);

    void terminationRequested( );

    ;
public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void startPrinting( );

    void tab_uiStateChanged( TabIndex const sender, UiState const state );
    void tabs_currentChanged( int index );
    void helpButton_clicked( bool );

    void printManager_printStarting( );
    void printManager_printComplete( bool const success );
    void printManager_printAborted( );

    void fileTab_modelSelected( ModelSelectionInfo const* modelSelection );

    void prepareTab_slicingNeeded( bool const needed );
    void prepareTab_preparePrinterStarted( );
    void prepareTab_preparePrinterComplete( bool const success );

    void signalHandler_signalReceived( siginfo_t const& info );

};

#endif // __WINDOW_H__
