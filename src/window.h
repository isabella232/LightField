#ifndef WINDOW_H
#define WINDOW_H

#include <QtDebug>

#include <QMainWindow>

#include <QFileSystemModel>
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

class Window: public QMainWindow {

    Q_OBJECT

public:

    Window(bool fullScreen, QWidget* parent=0);

    bool load_stl(const QString& filename);

protected:

    virtual void closeEvent( QCloseEvent* event ) override;
#if defined _DEBUG
    virtual void showEvent( QShowEvent* event ) override;
#endif

private:

    Shepherd*         shepherd                    { nullptr };
    Loader*           loader                      { nullptr };
    PrintManager*     printManager                { nullptr };
    PrintJob*         printJob                    { nullptr };
    QProcess*         slicerProcess               { nullptr };

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

    QWidget*          slicePlaceholder;
    QLabel*           layerThicknessLabel;
    QStringListModel* layerThicknessStringListModel;
    QListView*        layerThicknessListView;
    QVBoxLayout*      layerThicknessLayout;
    QWidget*          layerThicknessContainer;
    QPushButton*      sliceButton;
    QWidget*          sliceTab;
    QGridLayout*      sliceTabLayout;

    QWidget*          printPlaceholder;
    QLabel*           printLayerTimeLabel;
    QLineEdit*        printLayerTime;
    QLabel*           powerLevelLabel;
    QSlider*          powerLevelSlider;
    QLabel*           powerLevelSliderLeftLabel;
    QLabel*           powerLevelSliderRightLabel;
    QHBoxLayout*      powerLevelSliderLabelsLayout;
    QWidget*          powerLevelSliderLabelsContainer;
    QVBoxLayout*      printOptionsLayout;
    QWidget*          printOptionsContainer;
    QPushButton*      printButton;
    QWidget*          printTab;
    QGridLayout*      printTabLayout;

    QWidget*          progressPlaceholder;
    QWidget*          progressTab;
    QGridLayout*      progressTabLayout;

#if defined _DEBUG
    bool              hasBeenShown { false };
#endif

signals:

public slots:

protected slots:

private slots:

    void shepherd_Started( );
    void shepherd_Finished( int exitCode, QProcess::ExitStatus exitStatus );
    void shepherd_ProcessError( QProcess::ProcessError error );

    void printer_Online( );
    void printer_Offline( );
    void printer_Position( float position );
    void printer_Temperature( QString const& temperatureInfo );

    void printProcess_ShowImage( QString const& fileName, QString const& brightness, QString const& index, QString const& total );
    void printProcess_HideImage( );
    void printProcess_StartedPrinting( );
    void printProcess_FinishedPrinting( );

    void loader_ErrorBadStl();
    void loader_ErrorEmptyMesh();
    void loader_ErrorMissingFile();

    void loader_Finished();
    void loader_LoadedFile(QString const& filename);

    void tabs_currentChanged( int index );
    void tabs_tabBarClicked( int index );
    void tabs_tabBarDoubleClicked( int index );
    void tabs_tabCloseRequested( int index );

    void fileSystemModel_DirectoryLoaded( QString const& name );
    void fileSystemModel_FileRenamed( QString const& path, QString const& oldName, QString const& newName );
    void fileSystemModel_RootPathChanged( QString const& newPath );

    void availableFilesListView_clicked( QModelIndex const& index );
    void selectButton_clicked( bool checked );

    void layerThicknessListView_clicked( QModelIndex const& index );
    void sliceButton_clicked( bool checked );

    void printLayerTime_editingFinished( );
    void powerLevelSlider_valueChanged( int value );
    void printButton_clicked( bool checked );

    void slicerProcessErrorOccurred( QProcess::ProcessError error );
    void slicerProcessStarted( );
    void slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

};

#endif // WINDOW_H
