#ifndef WINDOW_H
#define WINDOW_H

#include "app.h"
#include "shepherd.h"
#include "printmanager.h"
#include "printjob.h"
#include "selecttab.h"
#include "preparetab.h"
#include "printtab.h"
#include "statustab.h"

class Window: public QMainWindow {

    Q_OBJECT

public:

    Window( QWidget* parent = nullptr );
    virtual ~Window( ) override;

protected:

    virtual void closeEvent( QCloseEvent* event ) override;

private:

    Shepherd*     shepherd     {                };
    PrintManager* printManager {                };
    PrintJob*     printJob     { new PrintJob   };

    QTabWidget*   tabs         { new QTabWidget };
    SelectTab*    selectTab    { new SelectTab  };
    PrepareTab*   prepareTab   { new PrepareTab };
    PrintTab*     printTab     { new PrintTab   };
    StatusTab*    statusTab    { new StatusTab  };

signals:

    void printJobChanged( PrintJob* newJob );

public slots:

protected slots:

private slots:

    void shepherd_Started( );
    void shepherd_Finished( int exitCode, QProcess::ExitStatus exitStatus );
    void shepherd_ProcessError( QProcess::ProcessError error );

    void selectTab_modelSelected( bool success, QString const& fileName );

    void prepareTab_sliceStarting( );
    void prepareTab_sliceComplete( bool success );
    void prepareTab_renderStarting( );
    void prepareTab_renderComplete( bool success );

    void printTab_printButtonClicked( );

    void statusTab_stopButtonClicked( );

    void signalHandler_quit( int signalNumber );

};

#endif // WINDOW_H
