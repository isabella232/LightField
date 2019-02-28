#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "coordinate.h"

class Shepherd;
class PrintManager;
class PrintJob;

//class WelcomeTab;
class SelectTab;
class PrepareTab;
class PrintTab;
class StatusTab;

enum class TabIndex {
    //Welcome,
    Select,
    Prepare,
    Print,
    Status,
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
    SelectTab*    selectTab;
    PrepareTab*   prepareTab;
    PrintTab*     printTab;
    StatusTab*    statusTab;

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

    void shepherd_raiseBuildPlatformMoveToComplete( bool const success );
    void shepherd_homeComplete( bool const success );
    void shepherd_lowerBuildPlatformMoveToComplete( bool const success );

    void tabs_currentChanged( int index );

    void selectTab_modelSelected( bool const success, QString const& fileName );
    void selectTab_modelDimensioned( size_t const vertexCount, Coordinate const sizeX, Coordinate const sizeY, Coordinate const sizeZ );

    void prepareTab_sliceStarted( );
    void prepareTab_sliceComplete( bool const success );
    void prepareTab_renderStarted( );
    void prepareTab_renderComplete( bool const success );
    void prepareTab_preparePrinterComplete( bool const success );

    void printTab_printButtonClicked( );

    void printTab_raiseBuildPlatform( );
    void printTab_homePrinter( );
    void printTab_lowerBuildPlatform( );

    void statusTab_stopButtonClicked( );
    void statusTab_cleanUpAfterPrint( );

    void signalHandler_signalReceived( int const signalNumber );

};

char const* ToString( TabIndex const index );

#endif // __WINDOW_H__
