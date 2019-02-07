#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

#include "window.h"

#include "printmanager.h"
#include "strings.h"

namespace {

    QString StlModelLibraryPath { "/home/lumen/Volumetric/model-library" };

    QStringList LayerThicknessStringList {
        "50 µm",
        "100 µm",
        "200 µm",
    };

    int LayerThicknessValues[] { 50, 100, 200 };

    class TabIndex {
    public:
        enum {
            Select,
            Slice,
            Print,
            Progress
        };
    };

    QString SlicerCommand { "/home/lumen/Volumetric/fstl/slicer-command" };

}

Window::Window(bool fullScreen, QWidget *parent): QMainWindow(parent) {
    QMargins emptyMargins { };

    move( { 0, 800 } );
    if ( fullScreen ) {
        showFullScreen( );
    } else {
        setFixedSize( 800, 480 );
    }

    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
    format.setVersion( 2, 1 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( format );

    printJob = new PrintJob;

    //
    // "Select" tab
    //

    fileSystemModel = new QFileSystemModel;
    fileSystemModel->setFilter( QDir::Files );
    fileSystemModel->setNameFilterDisables( false );
    fileSystemModel->setNameFilters( {
        {
            "*.stl",
        }
    } );
    fileSystemModel->setRootPath( StlModelLibraryPath );
    QObject::connect( fileSystemModel, &QFileSystemModel::directoryLoaded, this, &Window::fileSystemModel_DirectoryLoaded );
    QObject::connect( fileSystemModel, &QFileSystemModel::fileRenamed,     this, &Window::fileSystemModel_FileRenamed     );
    QObject::connect( fileSystemModel, &QFileSystemModel::rootPathChanged, this, &Window::fileSystemModel_RootPathChanged );

    availableFilesListView = new QListView;
    availableFilesListView->setFlow( QListView::TopToBottom );
    availableFilesListView->setLayoutMode( QListView::SinglePass );
    availableFilesListView->setMovement( QListView::Static );
    availableFilesListView->setResizeMode( QListView::Fixed );
    availableFilesListView->setViewMode( QListView::ListMode );
    availableFilesListView->setWrapping( true );
    availableFilesListView->setModel( fileSystemModel );
    QObject::connect( availableFilesListView, &QListView::clicked, this, &Window::availableFilesListView_clicked );

    availableFilesLabel = new QLabel( "Available files:" );
    availableFilesLabel->setBuddy( availableFilesListView );

    availableFilesLayout = new QGridLayout;
    availableFilesLayout->setContentsMargins( emptyMargins );
    availableFilesLayout->addWidget( availableFilesLabel,    0, 0 );
    availableFilesLayout->addWidget( availableFilesListView, 1, 0 );

    availableFilesContainer = new QWidget( );
    availableFilesContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    availableFilesContainer->setLayout( availableFilesLayout );

    selectButton = new QPushButton( "Select" );
    {
        auto font { selectButton->font( ) };
        font.setPointSizeF( 22.25 );
        selectButton->setFont( font );
    }
    selectButton->setEnabled( false );
    selectButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    QObject::connect( selectButton, &QPushButton::clicked, this, &Window::selectButton_clicked );

    canvas = new Canvas( format, this );
    canvas->setMinimumSize( 600, 400 );
    canvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    selectTabLayout = new QGridLayout;
    selectTabLayout->setContentsMargins( emptyMargins );
    selectTabLayout->addWidget( availableFilesContainer, 0, 0, 1, 1 );
    selectTabLayout->addWidget( selectButton,            1, 0, 1, 1 );
    selectTabLayout->addWidget( canvas,                  0, 1, 2, 1 );
    selectTabLayout->setRowStretch( 0, 4 );
    selectTabLayout->setRowStretch( 1, 1 );

    selectTab = new QWidget;
    selectTab->setContentsMargins( emptyMargins );
    selectTab->setLayout( selectTabLayout );
    selectTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "Slice" tab
    //

    layerThicknessStringListModel = new QStringListModel( LayerThicknessStringList );

    layerThicknessListView = new QListView;
    layerThicknessListView->setLayoutMode( QListView::SinglePass );
    layerThicknessListView->setMovement( QListView::Static );
    layerThicknessListView->setResizeMode( QListView::Fixed );
    layerThicknessListView->setFlow( QListView::TopToBottom );
    layerThicknessListView->setViewMode( QListView::ListMode );
    layerThicknessListView->setWrapping( true );

    layerThicknessListView->setModel( layerThicknessStringListModel );
    layerThicknessListView->setCurrentIndex( layerThicknessStringListModel->index( 1, 0 ) );
    printJob->layerThickness = 100;
    QObject::connect( layerThicknessListView, &QListView::clicked, this, &Window::layerThicknessListView_clicked );

    layerThicknessLabel = new QLabel( "Layer thickness:" );
    layerThicknessLabel->setBuddy( layerThicknessListView );

    layerThicknessLayout = new QVBoxLayout;
    layerThicknessLayout->setContentsMargins( emptyMargins );
    layerThicknessLayout->addWidget( layerThicknessLabel );
    layerThicknessLayout->addWidget( layerThicknessListView );
    layerThicknessLayout->addStretch( );

    layerThicknessContainer = new QWidget( );
    layerThicknessContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    layerThicknessContainer->setLayout( layerThicknessLayout );

    sliceButton = new QPushButton( "Slice" );
    {
        auto font { sliceButton->font( ) };
        font.setPointSizeF( 22.25 );
        sliceButton->setFont( font );
    }
    sliceButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    QObject::connect( sliceButton, &QPushButton::clicked, this, &Window::sliceButton_clicked );

    slicePlaceholder = new QWidget;
    slicePlaceholder->setMinimumSize( 600, 400 );
    slicePlaceholder->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    sliceTabLayout = new QGridLayout;
    sliceTabLayout->setContentsMargins( emptyMargins );
    sliceTabLayout->addWidget( layerThicknessContainer, 0, 0, 1, 1 );
    sliceTabLayout->addWidget( sliceButton,             1, 0, 1, 1 );
    sliceTabLayout->addWidget( slicePlaceholder,        0, 1, 2, 1 );
    sliceTabLayout->setRowStretch( 0, 4 );
    sliceTabLayout->setRowStretch( 1, 1 );

    sliceTab = new QWidget;
    sliceTab->setContentsMargins( emptyMargins );
    sliceTab->setLayout( sliceTabLayout );
    sliceTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "Print" tab
    //

    printLayerTime = new QLineEdit;
    printLayerTime->setAlignment( Qt::AlignRight );
    printLayerTime->setText( "1.0" );
    printLayerTime->setValidator( new QDoubleValidator( 0.0, 1.0E10, 10 ) );
    QObject::connect( printLayerTime, &QLineEdit::editingFinished, this, &Window::printLayerTime_editingFinished );

    printLayerTimeLabel = new QLabel( "Exposure time:" );
    printLayerTimeLabel->setBuddy( printLayerTime );

    powerLevelSlider = new QSlider( Qt::Orientation::Horizontal );
    powerLevelSlider->setTickPosition( QSlider::TickPosition::TicksBelow );
    powerLevelSlider->setMinimum( 20 );
    powerLevelSlider->setMaximum( 100 );
    powerLevelSlider->setValue( 50 );
    printJob->brightness = 127;
    QObject::connect( powerLevelSlider, &QSlider::valueChanged, this, &Window::powerLevelSlider_valueChanged );

    powerLevelLabel = new QLabel( "Projector power level:" );
    powerLevelLabel->setBuddy( powerLevelSlider );

    powerLevelSliderLeftLabel = new QLabel( "20%" );
    powerLevelSliderLeftLabel->setAlignment( Qt::AlignLeft );
    powerLevelSliderRightLabel = new QLabel( "100%" );
    powerLevelSliderRightLabel->setAlignment( Qt::AlignRight );

    powerLevelSliderLabelsLayout = new QHBoxLayout( );
    powerLevelSliderLabelsLayout->addWidget( powerLevelSliderLeftLabel );
    powerLevelSliderLabelsLayout->addStretch( );
    powerLevelSliderLabelsLayout->addWidget( powerLevelSliderRightLabel );

    powerLevelSliderLabelsContainer = new QWidget( );
    powerLevelSliderLabelsContainer->setLayout( powerLevelSliderLabelsLayout );

    printOptionsLayout = new QVBoxLayout;
    printOptionsLayout->setContentsMargins( emptyMargins );
    printOptionsLayout->addWidget( printLayerTimeLabel );
    printOptionsLayout->addWidget( printLayerTime );
    printOptionsLayout->addWidget( powerLevelLabel );
    printOptionsLayout->addWidget( powerLevelSlider );
    printOptionsLayout->addWidget( powerLevelSliderLabelsContainer );
    printOptionsLayout->addStretch( );

    printOptionsContainer = new QWidget( );
    printOptionsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    printOptionsContainer->setLayout( printOptionsLayout );

    printButton = new QPushButton( "Print" );
    {
        auto font { printButton->font( ) };
        font.setPointSizeF( 22.25 );
        printButton->setFont( font );
    }
    printButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    QObject::connect( printButton, &QPushButton::clicked, this, &Window::printButton_clicked );

    printPlaceholder = new QWidget;
    printPlaceholder->setMinimumSize( 600, 400 );
    printPlaceholder->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    printTabLayout = new QGridLayout;
    printTabLayout->setContentsMargins( emptyMargins );
    printTabLayout->addWidget( printOptionsContainer, 0, 0, 1, 1 );
    printTabLayout->addWidget( printButton,           1, 0, 1, 1 );
    printTabLayout->addWidget( printPlaceholder,      0, 1, 2, 1 );
    printTabLayout->setRowStretch( 0, 4 );
    printTabLayout->setRowStretch( 1, 1 );

    printTab = new QWidget;
    printTab->setContentsMargins( emptyMargins );
    printTab->setLayout( printTabLayout );
    printTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "Progress" tab
    //

    progressPlaceholder = new QWidget;
    progressPlaceholder->setMinimumSize( 600, 400 );
    progressPlaceholder->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    progressTabLayout = new QGridLayout;
    progressTabLayout->setContentsMargins( emptyMargins );
    progressTabLayout->addWidget( progressPlaceholder, 0, 1, 2, 1 );
    progressTabLayout->setRowStretch( 0, 4 );
    progressTabLayout->setRowStretch( 1, 1 );

    progressTab = new QWidget;
    progressTab->setContentsMargins( emptyMargins );
    progressTab->setLayout( progressTabLayout );
    progressTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // Tab widget
    //

    tabs = new QTabWidget;
    tabs->setContentsMargins( emptyMargins );
    tabs->addTab( selectTab,   "Select"   );
    tabs->addTab( sliceTab,    "Slice"    );
    tabs->addTab( printTab,    "Print"    );
    tabs->addTab( progressTab, "Progress" );
    tabs->setCurrentIndex( TabIndex::Select );

    setCentralWidget( tabs );

    shepherd = new Shepherd( parent );
    QObject::connect( shepherd, &Shepherd::shepherd_Started,              this, &Window::shepherd_Started              );
    QObject::connect( shepherd, &Shepherd::shepherd_Finished,             this, &Window::shepherd_Finished             );
    QObject::connect( shepherd, &Shepherd::shepherd_ProcessError,         this, &Window::shepherd_ProcessError         );

    QObject::connect( shepherd, &Shepherd::printer_Online,                this, &Window::printer_Online                );
    QObject::connect( shepherd, &Shepherd::printer_Offline,               this, &Window::printer_Offline               );
    QObject::connect( shepherd, &Shepherd::printer_Position,              this, &Window::printer_Position              );
    QObject::connect( shepherd, &Shepherd::printer_Temperature,           this, &Window::printer_Temperature           );

    QObject::connect( shepherd, &Shepherd::printProcess_ShowImage,        this, &Window::printProcess_ShowImage        );
    QObject::connect( shepherd, &Shepherd::printProcess_HideImage,        this, &Window::printProcess_HideImage        );
    QObject::connect( shepherd, &Shepherd::printProcess_StartedPrinting,  this, &Window::printProcess_StartedPrinting  );
    QObject::connect( shepherd, &Shepherd::printProcess_FinishedPrinting, this, &Window::printProcess_FinishedPrinting );
    shepherd->start( );
}

void Window::shepherd_Started( ) {
    fprintf( stderr, "+ Window::shepherd_Started\n" );
}

void Window::shepherd_Finished( int exitCode, QProcess::ExitStatus exitStatus ) {
    fprintf( stderr, "+ Window::shepherd_Finished: exitStatus %d, exitCode %d\n", exitStatus, exitCode );
}

void Window::shepherd_ProcessError( QProcess::ProcessError error ) {
    fprintf( stderr, "+ Window::shepherd_ProcessError: %d\n", error );
}

void Window::printer_Online( ) {
    fprintf( stderr, "+ Window::printer_Online\n" );
}

void Window::printer_Offline( ) {
    fprintf( stderr, "+ Window::printer_Offline\n" );
}

void Window::printer_Position( float position ) {
    fprintf( stderr, "+ Window::printer_Position: position: %f\n", position );
}

void Window::printer_Temperature( QString const& temperatureInfo ) {
    fprintf( stderr, "+ Window::printer_Temperature: temperatureInfo: '%s'\n", temperatureInfo.toUtf8( ).data( ) );
}

void Window::printProcess_ShowImage( QString const& fileName, QString const& brightness, QString const& index, QString const& total ) {
    fprintf( stderr, "+ Window::printProcess_ShowImage: fileName '%s', brightness %s, index %s, total %s\n", fileName.toUtf8( ).data( ), brightness.toUtf8( ).data( ), index.toUtf8( ).data( ), total.toUtf8( ).data( ) );
}

void Window::printProcess_HideImage( ) {
    fprintf( stderr, "+ Window::printProcess_HideImage\n" );
}

void Window::printProcess_StartedPrinting( ) {
    fprintf( stderr, "+ Window::printProcess_StartedPrinting\n" );
}

void Window::printProcess_FinishedPrinting( ) {
    fprintf( stderr, "+ Window::printProcess_FinishedPrinting\n" );
}

void Window::loader_ErrorBadStl()
{
    QMessageBox::critical(this, "Error",
                          "<b>Error:</b><br>"
                          "This <code>.stl</code> file is invalid or corrupted.<br>"
                          "Please export it from the original source, verify, and retry.");
}

void Window::loader_ErrorEmptyMesh()
{
    QMessageBox::critical(this, "Error",
                          "<b>Error:</b><br>"
                          "This file is syntactically correct<br>but contains no triangles.");
}

void Window::loader_ErrorMissingFile()
{
    QMessageBox::critical(this, "Error",
                          "<b>Error:</b><br>"
                          "The target file is missing.<br>");
}

void Window::loader_Finished( ) {
    loader = nullptr;
}

void Window::loader_LoadedFile(const QString& filename)
{
    fprintf( stderr, "+ Window::loader_LoadedFile: filename: '%s'\n", filename.toUtf8( ).data( ) );
    printJob->modelFileName = filename;
    selectButton->setEnabled( true );
}

void Window::closeEvent( QCloseEvent* event ) {
    fprintf( stderr, "+ Window::closeEvent\n" );
    shepherd->doTerminate( );
    if ( printManager ) {
        printManager->terminate( );
    }
    event->accept( );
}

#if defined _DEBUG
void Window::showEvent( QShowEvent* event ) {
    fprintf( stderr, "+ Window::showEvent\n" );
    if ( !hasBeenShown ) {
        PrintJob* printJob = new PrintJob;
        printJob->modelFileName     = "/home/lumen/Volumetric/model-library/makerook.stl";
        printJob->slicedSvgFileName = "";
        printJob->pngFilesPath      = "/home/lumen/Volumetric/model-library/makerook_imgs";
        printJob->layerCount        = 238;
        printJob->layerThickness    = 100;
        printJob->exposureTime      = 1.0;
        printJob->brightness        = 127;

        printManager = new PrintManager( shepherd, this );
        printManager->print( printJob );
    }
    event->accept( );
}
#endif // _DEBUG

void Window::tabs_currentChanged( int index ) {
    fprintf( stderr, "+ Window::tabs_currentChanged: index: %d\n", index );
}

void Window::tabs_tabBarClicked( int index ) {
    fprintf( stderr, "+ Window::tabs_tabBarClicked: index: %d\n", index );
}

void Window::tabs_tabBarDoubleClicked( int index ) {
    fprintf( stderr, "+ Window::tabs_tabBarDoubleClicked: index: %d\n", index );
}

void Window::tabs_tabCloseRequested( int index ) {
    fprintf( stderr, "+ Window::tabs_tabCloseRequested: index: %d\n", index );
}

void Window::fileSystemModel_DirectoryLoaded( QString const& name ) {
    fprintf( stderr, "+ Window::fileSystemModel_DirectoryLoaded: name '%s'\n", name.toUtf8( ).data( ) );
    fileSystemModel->sort( 0, Qt::AscendingOrder );

    auto index = fileSystemModel->index( StlModelLibraryPath );
    fprintf( stderr, "  + fileSystemModel->rootIndex() %d,%d\n", index.row( ), index.column( ) );
    availableFilesListView->setRootIndex( index );
}

void Window::fileSystemModel_FileRenamed( QString const& path, QString const& oldName, QString const& newName ) {
    fprintf( stderr, "+ Window::fileSystemModel_FileRenamed: path '%s', oldName '%s', newName '%s'\n", path.toUtf8( ).data( ), oldName.toUtf8( ).data( ), newName.toUtf8( ).data( ) );
}

void Window::fileSystemModel_RootPathChanged( QString const& newPath ) {
    fprintf( stderr, "+ Window::fileSystemModel_RootPathChanged: newPath '%s'\n", newPath.toUtf8( ).data( ) );
}

void Window::availableFilesListView_clicked( QModelIndex const& index ) {
    QString fileName = StlModelLibraryPath + QString( '/' ) + index.data( ).toString( );
    fprintf( stderr, "+ Window::availableFilesListView_clicked: row %d, file name '%s'\n", index.row( ), fileName.toUtf8( ).data( ) );
    if ( !load_stl( fileName ) ) {
        fprintf( stderr, "  + load_stl failed!\n" );
    }
}

void Window::selectButton_clicked( bool /*checked*/ ) {
    fprintf( stderr, "+ Window::selectButton_clicked\n" );
    printJob->pngFilesPath = StlModelLibraryPath + QString( "/%1" ).arg( getpid( ) * 100000 + rand( ) );
    tabs->setCurrentIndex( TabIndex::Slice );
}

void Window::layerThicknessListView_clicked( QModelIndex const& index ) {
    fprintf( stderr, "+ Window::layerThicknessListView_clicked: new value: %d µm\n", LayerThicknessValues[index.row( )] );
    printJob->layerThickness = LayerThicknessValues[index.row( )];
}

void Window::sliceButton_clicked( bool /*checked*/ ) {
    fprintf( stderr, "+ Window::sliceButton_clicked\n" );

    slicerProcess = new QProcess( this );
    slicerProcess->setProgram( SlicerCommand );
    slicerProcess->setArguments( QStringList {
        printJob->modelFileName,
        printJob->pngFilesPath
    } );
    QObject::connect( slicerProcess, &QProcess::errorOccurred, this, &Window::slicerProcessErrorOccurred );
    QObject::connect( slicerProcess, &QProcess::started,       this, &Window::slicerProcessStarted       );
    QObject::connect( slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Window::slicerProcessFinished );
    slicerProcess->start( );
}

void Window::printLayerTime_editingFinished( ) {
    bool valueOk = false;
    double value = printLayerTime->validator( )->locale( ).toDouble( printLayerTime->text( ), &valueOk );
    if ( valueOk ) {
        fprintf( stderr, "+ Window::printLayerTime_editingFinished: new value %f\n", value );
        printJob->exposureTime = value;
    } else {
        fprintf( stderr, "+ Window::printLayerTime_editingFinished: bad value\n" );
    }
}

void Window::powerLevelSlider_valueChanged( int value ) {
    fprintf( stderr, "+ Window::powerLevelSlider_valueChanged: value %d%%\n", value );
    printJob->brightness = value * 255 / 100;
}

void Window::printButton_clicked( bool /*checked*/ ) {
    fprintf( stderr, "+ Window::printButton_clicked\n" );
    tabs->setCurrentIndex( TabIndex::Progress );

    printJob->pngFilesPath = "/home/lumen/Volumetric/model-library/makerook_imgs";
    printJob->layerCount   = 238;

    fprintf( stderr,
        "  + Print job:\n"
        "    + modelFileName:     '%s'\n"
        "    + slicedSvgFileName: '%s'\n"
        "    + pngFilesPath:      '%s'\n"
        "    + layerCount:        %d\n"
        "    + layerThickness:    %d\n"
        "    + exposureTime:      %f\n"
        "    + brightness:        %d\n"
        "",
        printJob->modelFileName.toUtf8( ).data( ),
        printJob->slicedSvgFileName.toUtf8( ).data( ),
        printJob->pngFilesPath.toUtf8( ).data( ),
        printJob->layerCount,
        printJob->layerThickness,
        printJob->exposureTime,
        printJob->brightness
    );

    printManager = new PrintManager( shepherd, this );
    printManager->print( printJob );

    printJob = new PrintJob;
}

void Window::slicerProcessErrorOccurred( QProcess::ProcessError error ) {
    QObject::disconnect( slicerProcess, &QProcess::errorOccurred, this, &Window::slicerProcessErrorOccurred );
    QObject::disconnect( slicerProcess, &QProcess::started,       this, &Window::slicerProcessStarted       );
    QObject::disconnect( slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Window::slicerProcessFinished );

    fprintf( stderr, "+ Window::slicerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + setpower process failed to start\n" );
    } else if ( QProcess::Crashed == error ) {
        fprintf( stderr, "  + setpower process crashed?\n" );
        if ( slicerProcess->state( ) != QProcess::NotRunning ) {
            slicerProcess->terminate( );
            slicerProcess->waitForFinished( );
        }
    }

    delete slicerProcess;
    slicerProcess = nullptr;
}

void Window::slicerProcessStarted( ) {
    fprintf( stderr, "+ Window::slicerProcessStarted\n" );
}

void Window::slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    QObject::disconnect( slicerProcess, &QProcess::errorOccurred, this, &Window::slicerProcessErrorOccurred );
    QObject::disconnect( slicerProcess, &QProcess::started,       this, &Window::slicerProcessStarted       );
    QObject::disconnect( slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Window::slicerProcessFinished );

    fprintf( stderr, "+ Window::slicerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    delete slicerProcess;
    slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        fprintf( stderr, "  + setpower process crashed?\n" );
        return;
    }

    tabs->setCurrentIndex( +TabIndex::Print );
}

bool Window::load_stl( QString const& filename ) {
    if (loader) {
        fprintf( stderr, "+ Window::load_stl: loader object exists, not loading\n" );
        return false;
    }

    canvas->set_status("Loading " + filename);

    loader = new Loader(this, filename, false);
    connect(loader, &Loader::got_mesh,           canvas, &Canvas::load_mesh);
    connect(loader, &Loader::error_bad_stl,      this,   &Window::loader_ErrorBadStl);
    connect(loader, &Loader::error_empty_mesh,   this,   &Window::loader_ErrorEmptyMesh);
    connect(loader, &Loader::error_missing_file, this,   &Window::loader_ErrorMissingFile);
    connect(loader, &Loader::finished,           loader, &Loader::deleteLater);
    connect(loader, &Loader::finished,           canvas, &Canvas::clear_status);
    connect(loader, &Loader::finished,           this,   &Window::loader_Finished);

    if (filename[0] != ':') {
        connect(loader, &Loader::loaded_file,    this,   &Window::setWindowTitle);
        connect(loader, &Loader::loaded_file,    this,   &Window::loader_LoadedFile);
    }

    loader->start();
    return true;
}
