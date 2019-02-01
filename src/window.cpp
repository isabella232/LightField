#include "window.h"

#include <cstdlib>

namespace {

    QString const StlModelLibraryPath = getenv( "DEBUGGING_ON_VIOLET" ) ? "/home/icekarma/devel/work/VolumetricLumen/fstl/model-library" : "/home/lumen/Volumetric/module-library";

}

Window::Window(QWidget *parent): QMainWindow(parent) {
    QMargins emptyMargins { };

    setFixedSize( 800, 480 );
    move( { 0, 0 } );

    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
    format.setVersion( 2, 1 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( format );

    //
    // "Select" tab
    //

    fileSystemModel = new QFileSystemModel;
    QObject::connect( fileSystemModel, &QFileSystemModel::directoryLoaded, this, &Window::fileSystemModel_DirectoryLoaded );
    QObject::connect( fileSystemModel, &QFileSystemModel::fileRenamed,     this, &Window::fileSystemModel_FileRenamed     );
    QObject::connect( fileSystemModel, &QFileSystemModel::rootPathChanged, this, &Window::fileSystemModel_RootPathChanged );
    fileSystemModel->setFilter( QDir::Files );
    fileSystemModel->setNameFilterDisables( false );
    fileSystemModel->setNameFilters( {
        {
            "*.stl",
        }
    } );
    fileSystemModel->setRootPath( StlModelLibraryPath );

    availableFilesListView = new QListView;
    QObject::connect( availableFilesListView, &QListView::clicked, this, &Window::availableFilesListView_clicked );
    availableFilesListView->setFlow( QListView::TopToBottom );
    availableFilesListView->setLayoutMode( QListView::SinglePass );
    availableFilesListView->setMovement( QListView::Static );
    availableFilesListView->setResizeMode( QListView::Fixed );
    availableFilesListView->setViewMode( QListView::ListMode );
    availableFilesListView->setWrapping( true );
    availableFilesListView->setModel( fileSystemModel );

    selectButton = new QPushButton( "Select" );
    QObject::connect( selectButton, &QPushButton::clicked, this, &Window::selectButton_clicked );
    {
        auto font { selectButton->font( ) };
        font.setPointSizeF( 22.25 );
        selectButton->setFont( font );
    }
    selectButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );

    canvas = new Canvas( format, this );
    canvas->setMinimumSize( 600, 400 );
    canvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    selectTabLayout = new QGridLayout;
    selectTabLayout->setContentsMargins( emptyMargins );
    selectTabLayout->addWidget( availableFilesListView, 0, 0, 1, 1 );
    selectTabLayout->addWidget( selectButton,           1, 0, 1, 1 );
    selectTabLayout->addWidget( canvas,                 0, 1, 2, 1 );
    selectTabLayout->setRowStretch( 0, 4 );
    selectTabLayout->setRowStretch( 1, 1 );

    selectTab = new QWidget;
    selectTab->setContentsMargins( emptyMargins );
    selectTab->setLayout( selectTabLayout );
    selectTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "Slice" tab
    //

    printQualityStringListModel = new QStringListModel( QStringList {
        {
            "50 µm",
            "100 µm",
            "200 µm",
        }
    } );

    printQualityListView = new QListView;
    QObject::connect( printQualityListView, &QListView::clicked, this, &Window::printQualityListView_clicked );
    printQualityListView->setFlow( QListView::TopToBottom );
    printQualityListView->setLayoutMode( QListView::SinglePass );
    printQualityListView->setMovement( QListView::Static );
    printQualityListView->setResizeMode( QListView::Fixed );
    printQualityListView->setViewMode( QListView::ListMode );
    printQualityListView->setWrapping( true );
    printQualityListView->setModel( printQualityStringListModel );

    sliceButton = new QPushButton( "Slice" );
    QObject::connect( sliceButton, &QPushButton::clicked, this, &Window::sliceButton_clicked );
    {
        auto font { sliceButton->font( ) };
        font.setPointSizeF( 22.25 );
        sliceButton->setFont( font );
    }
    sliceButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );

    slicePlaceholder = new QWidget;
    slicePlaceholder->setMinimumSize( 600, 400 );
    slicePlaceholder->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    sliceTabLayout = new QGridLayout;
    sliceTabLayout->setContentsMargins( emptyMargins );
    sliceTabLayout->addWidget( printQualityListView, 0, 0, 1, 1 );
    sliceTabLayout->addWidget( sliceButton,          1, 0, 1, 1 );
    sliceTabLayout->addWidget( slicePlaceholder,     0, 1, 2, 1 );
    sliceTabLayout->setRowStretch( 0, 4 );
    sliceTabLayout->setRowStretch( 1, 1 );

    sliceTab = new QWidget;
    sliceTab->setContentsMargins( emptyMargins );
    sliceTab->setLayout( sliceTabLayout );
    sliceTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "Print" tab
    //

    printLayerTime = new QTextEdit;
    printLayerTime->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    printLayerTime->setAcceptDrops( false );
    printLayerTime->setAcceptRichText( false );
    printLayerTime->setAutoFormatting( QTextEdit::AutoFormattingFlag::AutoNone );
    printLayerTime->setOverwriteMode( false );
    printLayerTime->setReadOnly( false );
    printLayerTime->setTabChangesFocus( true );
    printLayerTime->setTextInteractionFlags( Qt::TextEditorInteraction );
    printLayerTime->setUndoRedoEnabled( true );
    printLayerTime->setWordWrapMode( QTextOption::NoWrap );

    printButton = new QPushButton( "Print" );
    QObject::connect( printButton, &QPushButton::clicked, this, &Window::printButton_clicked );
    {
        auto font { printButton->font( ) };
        font.setPointSizeF( 22.25 );
        printButton->setFont( font );
    }
    printButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );

    printPlaceholder = new QWidget;
    printPlaceholder->setMinimumSize( 600, 400 );
    printPlaceholder->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    printTabLayout = new QGridLayout;
    printTabLayout->setContentsMargins( emptyMargins );
    printTabLayout->addWidget( printLayerTime,       0, 0, 1, 1 );
    printTabLayout->addWidget( printButton,          1, 0, 1, 1 );
    printTabLayout->addWidget( printPlaceholder,     0, 1, 2, 1 );
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
    progressTabLayout->addWidget( progressPlaceholder,     0, 1, 2, 1 );
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
    tabs->setCurrentIndex( 0 );

    setCentralWidget( tabs );

    shepherd = new Shepherd( this );
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

void Window::printer_Temperature( char const* temperatureInfo ) {
    fprintf( stderr, "+ Window::printer_Temperature: temperatureInfo: '%s'\n", temperatureInfo );
}

void Window::printProcess_ShowImage( char const* fileName, char const* brightness, char const* index, char const* total ) {
    fprintf( stderr, "+ Window::printProcess_ShowImage: fileName '%s', brightness %s, index %s, total %s\n", fileName, brightness, index, total );
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
    currentFileName = filename;
}

void Window::closeEvent( QCloseEvent* event ) {
    fprintf( stderr, "+ Window::closeEvent\n" );
    shepherd->terminate( );
    event->accept( );
}

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
    QString fileName = StlModelLibraryPath + '/' + index.data( ).toString( );
    fprintf( stderr, "+ Window::availableFilesListView_clicked:\n" );
    fprintf( stderr, "  + row %d, selected file name: %s\n", index.row( ), fileName.toUtf8( ).data( ) );
    if ( !load_stl( StlModelLibraryPath + '/' + index.data( ).toString( ) ) ) {
        fprintf( stderr, "  + load_stl failed!\n" );
    }
}

void Window::selectButton_clicked( bool /*checked*/ ) {
    fprintf( stderr, "+ Window::selectButton_clicked\n" );
}

void Window::printQualityListView_clicked( QModelIndex const& /*index*/ ) {
    fprintf( stderr, "+ Window::printQualityListView_clicked\n" );
}

void Window::sliceButton_clicked( bool /*checked*/ ) {
    fprintf( stderr, "+ Window::sliceButton_clicked\n" );
}

void Window::printButton_clicked( bool /*checked*/ ) {
    fprintf( stderr, "+ Window::printButton_clicked\n" );
}

bool Window::load_stl( QString const& filename ) {
    fprintf( stderr, "+ Window::load_stl: loader %p\n", loader );

    if (loader) {
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

void Window::setFullScreen(bool const fullScreen)
{
    if (fullScreen) {
        setWindowState(windowState() |  Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}
