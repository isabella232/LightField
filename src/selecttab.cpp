#include "pch.h"

#include "selecttab.h"

#include "canvas.h"
#include "loader.h"
#include "printjob.h"

namespace {

    QString DefaultModelFileName { ":gl/BoundingBox.stl" };

}

SelectTab::SelectTab( QWidget* parent ): QWidget( parent ) {
    debug( "SelectTab::`ctor: creating SelectTab instance at %p; parent: %p\n", this, parent );

    _fileSystemModel->setFilter( QDir::Files );
    _fileSystemModel->setNameFilterDisables( false );
    _fileSystemModel->setNameFilters( {
        {
            "*.stl",
        }
    } );
    _fileSystemModel->setRootPath( StlModelLibraryPath );
    QObject::connect( _fileSystemModel, &QFileSystemModel::directoryLoaded, this, &SelectTab::fileSystemModel_DirectoryLoaded );
    QObject::connect( _fileSystemModel, &QFileSystemModel::fileRenamed,     this, &SelectTab::fileSystemModel_FileRenamed     );
    QObject::connect( _fileSystemModel, &QFileSystemModel::rootPathChanged, this, &SelectTab::fileSystemModel_RootPathChanged );

    _availableFilesListView->setFlow( QListView::TopToBottom );
    _availableFilesListView->setLayoutMode( QListView::SinglePass );
    _availableFilesListView->setMovement( QListView::Static );
    _availableFilesListView->setResizeMode( QListView::Fixed );
    _availableFilesListView->setViewMode( QListView::ListMode );
    _availableFilesListView->setWrapping( true );
    _availableFilesListView->setModel( _fileSystemModel );
    QObject::connect( _availableFilesListView, &QListView::clicked, this, &SelectTab::availableFilesListView_clicked );

    _availableFilesLabel->setText( "Available files:" );
    _availableFilesLabel->setBuddy( _availableFilesListView );

    _availableFilesLayout->setContentsMargins( { } );
    _availableFilesLayout->addWidget( _availableFilesLabel,    0, 0 );
    _availableFilesLayout->addWidget( _availableFilesListView, 1, 0 );

    _availableFilesContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _availableFilesContainer->setLayout( _availableFilesLayout );

    {
        auto font { _selectButton->font( ) };
        font.setPointSizeF( 22.25 );
        _selectButton->setFont( font );
    }
    _selectButton->setText( "Select" );
    _selectButton->setEnabled( false );
    _selectButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    QObject::connect( _selectButton, &QPushButton::clicked, this, &SelectTab::selectButton_clicked );

    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
    format.setVersion( 2, 1 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( format );

    _canvas = new Canvas( format, this );
    _canvas->setMinimumSize( MaximalRightHandPaneSize );
    _canvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    _layout->setContentsMargins( { } );
    _layout->addWidget( _availableFilesContainer, 0, 0, 1, 1 );
    _layout->addWidget( _selectButton,            1, 0, 1, 1 );
    _layout->addWidget( _canvas,                  0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );

    _loadModel( DefaultModelFileName );
}

SelectTab::~SelectTab( ) {
    debug( "SelectTab::`dtor: destroying SelectTab instance at %p\n", this );
}

void SelectTab::fileSystemModel_DirectoryLoaded( QString const& name ) {
    debug( "+ SelectTab::fileSystemModel_DirectoryLoaded: name '%s'\n", name.toUtf8( ).data( ) );
    _fileSystemModel->sort( 0, Qt::AscendingOrder );
    _availableFilesListView->setRootIndex( _fileSystemModel->index( StlModelLibraryPath ) );
}

void SelectTab::fileSystemModel_FileRenamed( QString const& path, QString const& oldName, QString const& newName ) {
    debug( "+ SelectTab::fileSystemModel_FileRenamed: path '%s', oldName '%s', newName '%s'\n", path.toUtf8( ).data( ), oldName.toUtf8( ).data( ), newName.toUtf8( ).data( ) );
}

void SelectTab::fileSystemModel_RootPathChanged( QString const& newPath ) {
    debug( "+ SelectTab::fileSystemModel_RootPathChanged: newPath '%s'\n", newPath.toUtf8( ).data( ) );
}

void SelectTab::availableFilesListView_clicked( QModelIndex const& index ) {
    _fileName = StlModelLibraryPath + QString( '/' ) + index.data( ).toString( );
    debug( "+ SelectTab::availableFilesListView_clicked: row %d, file name '%s'\n", index.row( ), _fileName.toUtf8( ).data( ) );
    _selectButton->setEnabled( false );
    if ( !_loadModel( _fileName ) ) {
        debug( "  + _loadModel failed!\n" );
    }
}

void SelectTab::selectButton_clicked( bool /*checked*/ ) {
    debug( "+ SelectTab::selectButton_clicked\n" );
    emit modelSelected( true, _fileName );
}

void SelectTab::loader_ErrorBadStl( ) {
    QMessageBox::critical( this, "Error",
        "<b>Error:</b><br>"
        "This <code>.stl</code> file is invalid or corrupted.<br>"
        "Please export it from the original source, verify, and retry."
    );
    emit modelSelected( false, _fileName );
}

void SelectTab::loader_ErrorEmptyMesh( ) {
    QMessageBox::critical( this, "Error",
        "<b>Error:</b><br>"
        "This file is syntactically correct<br>but contains no triangles."
    );
    emit modelSelected( false, _fileName );
}

void SelectTab::loader_ErrorMissingFile( ) {
    QMessageBox::critical( this, "Error",
        "<b>Error:</b><br>"
        "The target file is missing.<br>"
    );
    emit modelSelected( false, _fileName );
}

void SelectTab::loader_Finished( ) {
    _loader = nullptr;
}

void SelectTab::loader_LoadedFile( const QString& fileName ) {
    debug( "+ SelectTab::loader_LoadedFile: filename: '%s'\n", fileName.toUtf8( ).data( ) );
    _selectButton->setEnabled( true );
}

bool SelectTab::_loadModel( QString const& fileName ) {
    if ( _loader ) {
        debug( "+ SelectTab::_loadModel: loader object exists, not loading\n" );
        return false;
    }

    _canvas->set_status( "Loading " + fileName );

    _loader = new Loader( this, fileName, false );
    connect( _loader, &Loader::got_mesh,           _canvas, &Canvas::load_mesh                  );
    connect( _loader, &Loader::error_bad_stl,      this,    &SelectTab::loader_ErrorBadStl      );
    connect( _loader, &Loader::error_empty_mesh,   this,    &SelectTab::loader_ErrorEmptyMesh   );
    connect( _loader, &Loader::error_missing_file, this,    &SelectTab::loader_ErrorMissingFile );
    connect( _loader, &Loader::finished,           _loader, &Loader::deleteLater                );
    connect( _loader, &Loader::finished,           _canvas, &Canvas::clear_status               );
    connect( _loader, &Loader::finished,           this,    &SelectTab::loader_Finished         );

    if ( fileName[0] != ':' ) {
        connect( _loader, &Loader::loaded_file,    this,    &SelectTab::loader_LoadedFile       );
    }

    _selectButton->setEnabled( false );
    _loader->start( );
    return true;
}

void SelectTab::setPrintJob( PrintJob* printJob ) {
    _printJob = printJob;
}
