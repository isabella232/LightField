#ifndef WINDOW_H
#define WINDOW_H

#include <QtDebug>

#include <QMainWindow>

#include <QFileSystemModel>
#include <QFormLayout>
#include <QListView>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QStringListModel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "shepherd.h"
#include "canvas.h"
#include "loader.h"
#include "printmanager.h"
#include "svgrenderer.h"
#include "debug.h"

class Window: public QMainWindow {

    Q_OBJECT

public:

    Window(bool fullScreen, bool debuggingPosition, QWidget* parent=0);
    virtual ~Window( ) override;

    bool load_stl(const QString& filename);

protected:

    virtual void closeEvent( QCloseEvent* event ) override;

private:

    Shepherd*         shepherd      { nullptr };
    Loader*           loader        { nullptr };
    PrintManager*     printManager  { nullptr };
    PrintJob*         printJob      { nullptr };
    QProcess*         slicerProcess { nullptr };
    SvgRenderer*      svgRenderer   { nullptr };

    bool              isPrinterOnline { false };

    QTabWidget*       tabs;

    Canvas*           canvas;
    QLabel*           availableFilesLabel;
    QFileSystemModel* fileSystemModel;
    QListView*        availableFilesListView;
    QGridLayout*      availableFilesLayout;
    QWidget*          availableFilesContainer;
    QPushButton*      selectButton;
    QWidget*          selectTab;
    QGridLayout*      selectTabLayout;

    QLabel*           sliceProgress;
    QLabel*           renderProgress;
    QFormLayout*      sliceProgressLayout;
    QWidget*          sliceProgressContainer;
    QLabel*           layerThicknessLabel;
    QStringListModel* layerThicknessStringListModel;
    QListView*        layerThicknessListView;
    QLabel*           exposureTimeLabel;
    QLineEdit*        exposureTime;
    QLabel*           powerLevelLabel;
    QSlider*          powerLevelSlider;
    QLabel*           powerLevelSliderLeftLabel;
    QLabel*           powerLevelSliderRightLabel;
    QHBoxLayout*      powerLevelSliderLabelsLayout;
    QWidget*          powerLevelSliderLabelsContainer;
    QVBoxLayout*      optionsLayout;
    QWidget*          optionsContainer;
    QPushButton*      sliceButton;
    QPushButton*      printButton;
    QLabel*           currentSliceLabel;
    QLabel*           currentSliceDisplay;
    QVBoxLayout*      currentSliceLayout;
    QWidget*          currentSliceContainer;
    QWidget*          printTab;
    QGridLayout*      printTabLayout;

    QLabel*           printerStateLabel;
    QLabel*           printerStateDisplay;
    QLabel*           projectorLampStateLabel;
    QLabel*           projectorLampStateDisplay;
    QLabel*           jobStateLabel;
    QLabel*           jobStateDisplay;
    QLabel*           currentLayerLabel;
    QLabel*           currentLayerDisplay;
    QVBoxLayout*      progressControlsLayout;
    QWidget*          progressControlsContainer;
    QLabel*           currentLayerImageLabel;
    QLabel*           currentLayerImageDisplay;
    QVBoxLayout*      currentLayerImageLayout;
    QWidget*          currentLayerImageContainer;
    QPushButton*      stopButton;
    QGridLayout*      statusTabLayout;
    QWidget*          statusTab;

signals:

public slots:

protected slots:

private slots:

    void shepherd_Started( );
    void shepherd_Finished( int exitCode, QProcess::ExitStatus exitStatus );
    void shepherd_ProcessError( QProcess::ProcessError error );

    void printer_Online( );
    void printer_Offline( );

    void loader_ErrorBadStl();
    void loader_ErrorEmptyMesh();
    void loader_ErrorMissingFile();

    void loader_Finished();
    void loader_LoadedFile(QString const& filename);

    void fileSystemModel_DirectoryLoaded( QString const& name );
    void fileSystemModel_FileRenamed( QString const& path, QString const& oldName, QString const& newName );
    void fileSystemModel_RootPathChanged( QString const& newPath );

    void availableFilesListView_clicked( QModelIndex const& index );
    void selectButton_clicked( bool checked );

    void layerThicknessListView_clicked( QModelIndex const& index );
    void exposureTime_editingFinished( );
    void powerLevelSlider_valueChanged( int value );
    void sliceButton_clicked( bool checked );
    void printButton_clicked( bool checked );

    void stopButton_clicked( bool checked );

    void printManager_printStarting( );
    void printManager_printingLayer( int layer );
    void printManager_lampStatusChange( bool const on );
    void printManager_printComplete( bool success );

    void slicerProcessErrorOccurred( QProcess::ProcessError error );
    void slicerProcessStarted( );
    void slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void svgRenderer_progress( int const currentLayer );
    void svgRenderer_done( int const totalLayers );

    void signalHandler_quit( int signalNumber );

};

#endif // WINDOW_H
