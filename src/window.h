#ifndef __WINDOW_H__
#define __WINDOW_H__

class Shepherd;
class PrintManager;
class PrintJob;

class SelectTab;
class PrepareTab;
class CalibrationTab;
class PrintTab;
class StatusTab;

class Window: public QMainWindow {

    Q_OBJECT

public:

    Window( QWidget* parent = nullptr );
    virtual ~Window( ) override;

protected:

    virtual void closeEvent( QCloseEvent* event ) override;

private:

    Shepherd*       shepherd;
    PrintManager*   printManager   { };
    PrintJob*       printJob;

    QTabWidget*     tabs           { new QTabWidget };
    SelectTab*      selectTab;
    PrepareTab*     prepareTab;
    CalibrationTab* calibrationTab;
    PrintTab*       printTab;
    StatusTab*      statusTab;

signals:

    void printJobChanged( PrintJob* newJob );

public slots:

protected slots:

private slots:

    void shepherd_started( );
    void shepherd_finished( int exitCode, QProcess::ExitStatus exitStatus );
    void shepherd_processError( QProcess::ProcessError error );

    void shepherd_adjustBedHeightMoveToComplete( bool const success );
    void shepherd_retractBuildPlatformMoveToComplete( bool const success );
    void shepherd_extendBuildPlatformMoveToComplete( bool const success );
    void shepherd_moveBuildPlatformUpMoveComplete( bool const success );
    void shepherd_moveBuildPlatformDownMoveComplete( bool const success );

    void selectTab_modelSelected( bool const success, QString const& fileName );

    void prepareTab_sliceStarted( );
    void prepareTab_sliceComplete( bool const success );
    void prepareTab_renderStarted( );
    void prepareTab_renderComplete( bool const success );

    void calibrationTab_calibrationStarted( );
    void calibrationTab_calibrationComplete( bool const success );

    void printTab_printButtonClicked( );

    void printTab_adjustBedHeight( double const newHeight );
    void printTab_retractBuildPlatform( );
    void printTab_extendBuildPlatform( );
    void printTab_moveBuildPlatformUp( );
    void printTab_moveBuildPlatformDown( );

    void statusTab_stopButtonClicked( );
    void statusTab_cleanUpAfterPrint( );

    void signalHandler_signalReceived( int const signalNumber );

};

#endif // __WINDOW_H__
