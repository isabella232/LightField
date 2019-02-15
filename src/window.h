#ifndef WINDOW_H
#define WINDOW_H

class Shepherd;
class PrintManager;
class PrintJob;
class SelectTab;
class PrepareTab;
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

    Shepherd*     shepherd;
    PrintManager* printManager { };
    PrintJob*     printJob;

    QTabWidget*   tabs         { new QTabWidget };
    SelectTab*    selectTab;
    PrepareTab*   prepareTab;
    PrintTab*     printTab;
    StatusTab*    statusTab;

signals:

    void printJobChanged( PrintJob* newJob );

public slots:

protected slots:

private slots:

    void shepherd_started( );
    void shepherd_finished( int exitCode, QProcess::ExitStatus exitStatus );
    void shepherd_processError( QProcess::ProcessError error );

    void shepherd_adjustBedHeightMoveToComplete( bool success );
    void shepherd_retractGewgawMoveToComplete( bool success );
    void shepherd_extendGewgawMoveToComplete( bool success );
    void shepherd_moveGewgawUpMoveComplete( bool success );
    void shepherd_moveGewgawDownMoveComplete( bool success );

    void selectTab_modelSelected( bool success, QString const& fileName );

    void prepareTab_sliceStarting( );
    void prepareTab_sliceComplete( bool success );
    void prepareTab_renderStarting( );
    void prepareTab_renderComplete( bool success );

    void printTab_printButtonClicked( );
    void printTab_adjustBedHeight( double const newHeight );
    void printTab_retractGewgaw( );
    void printTab_extendGewgaw( );
    void printTab_moveGewgawUp( );
    void printTab_moveGewgawDown( );

    void statusTab_stopButtonClicked( );
    void statusTab_cleanUpAfterPrint( );

    void signalHandler_quit( int signalNumber );

};

#endif // WINDOW_H
