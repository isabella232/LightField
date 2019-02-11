#ifndef WINDOW_H
#define WINDOW_H

#include "shepherd.h"
#include "canvas.h"
#include "loader.h"
#include "printmanager.h"
#include "svgrenderer.h"
#include "debug.h"
#include "selecttab.h"
#include "preparetab.h"
#include "printtab.h"
#include "statustab.h"

class Window: public QMainWindow {

    Q_OBJECT

public:

    Window(bool fullScreen, bool debuggingPosition, QWidget* parent=0);
    virtual ~Window( ) override;

protected:

    virtual void closeEvent( QCloseEvent* event ) override;

private:

    Shepherd*         shepherd      { nullptr };
    PrintManager*     printManager  { nullptr };
    PrintJob*         printJob      { nullptr };
    QProcess*         slicerProcess { nullptr };
    SvgRenderer*      svgRenderer   { nullptr };

    bool              isPrinterOnline { false };

    QTabWidget*       tabs;
    SelectTab*        selectTab;
    //PrepareTab*     prepareTab;
    //PrintTab*       printTab;
    //StatusTab*      statusTab;

    QLabel*           sliceProgress;
    QLabel*           renderProgress;
    QFormLayout*      sliceProgressLayout;
    QWidget*          sliceProgressContainer;
    QLabel*           layerThicknessLabel;
    QComboBox*        layerThicknessComboBox;
    QLabel*           exposureTimeLabel;
    QLineEdit*        exposureTime;
    QLabel*           exposureScaleFactorLabel;
    QComboBox*        exposureScaleFactorComboBox;
    QLabel*           powerLevelLabel;
    QLabel*           powerLevelValue;
    QHBoxLayout*      powerLevelValueLayout;
    QWidget*          powerLevelValueContainer;
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

    void selectTab_modelLoadComplete( bool const success, QString const& fileName );

    void layerThicknessComboBox_currentIndexChanged( int index );
    void exposureTime_editingFinished( );
    void exposureScaleFactorComboBox_currentIndexChanged( int index );
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
