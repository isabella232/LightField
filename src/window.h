#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "coordinate.h"

class ModelSelectionInfo;
class Shepherd;
class PrintManager;
class PrintJob;

//class WelcomeTab;
class FileTab;
class PrepareTab;
class PrintTab;
class StatusTab;
class AdvancedTab;

enum class TabIndex {
    //Welcome,
    File,
    Prepare,
    Print,
    Status,
    Advanced,
};

inline int operator+( TabIndex value ) {
    return static_cast<int>( value );
}

class Window: public QMainWindow {

    Q_OBJECT

public:

    Window( QWidget* parent = nullptr );
    virtual ~Window( ) override;

protected:

    virtual void closeEvent( QCloseEvent* event ) override;
    virtual void showEvent( QShowEvent* event ) override;

private:

    PrintJob*     printJob;
    PrintManager* printManager       { };
    Shepherd*     shepherd;

    QTabWidget*   tabs               { new QTabWidget };
    //WelcomeTab*   welcomeTab;
    FileTab*      fileTab;
    PrepareTab*   prepareTab;
    PrintTab*     printTab;
    StatusTab*    statusTab;
    AdvancedTab*  advancedTab;

    bool          _isPrinterPrepared { };
    bool          _isModelRendered   { };

    std::function<void( )> _initialShowEventFunc;

    void _initialShowEvent( );

signals:

    void printJobChanged( PrintJob* newJob );
    void shepherdChanged( Shepherd* newShepherd );

public slots:

protected slots:

private slots:

    void shepherd_started( );
    void shepherd_startFailed( );
    void shepherd_terminated( bool const expected, bool const cleanExit );

    void tabs_currentChanged( int index );

    void fileTab_modelSelected( ModelSelectionInfo* modelSelection );
    void fileTab_modelSelectionFailed( );

    void prepareTab_sliceStarted( );
    void prepareTab_sliceComplete( bool const success );
    void prepareTab_renderStarted( );
    void prepareTab_renderComplete( bool const success );
    void prepareTab_preparePrinterStarted( );
    void prepareTab_preparePrinterComplete( bool const success );

    void printTab_printButtonClicked( );

    void statusTab_stopButtonClicked( );
    void statusTab_cleanUpAfterPrint( );

    void signalHandler_signalReceived( int const signalNumber );

};

char const* ToString( TabIndex const index );

#endif // __WINDOW_H__
