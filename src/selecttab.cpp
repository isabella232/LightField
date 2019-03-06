#include "pch.h"

#include "selecttab.h"

#include "canvas.h"
#include "loader.h"
#include "mesh.h"
#include "printjob.h"
#include "processrunner.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"

namespace {

    QRegularExpression VolumeLineMatcher { QString { "^volume\\s*[:=]\\s*(\\d+(?:\\.(?:\\d+))?)" }, QRegularExpression::CaseInsensitiveOption };

}

SelectTab::SelectTab( QWidget* parent ): QWidget( parent ) {
    debug( "+ SelectTab::`ctor: construct at %p\n", this );

    _userMediaPath = MediaRootPath + Slash + GetUserName( );
    debug( "  + user media path '%s'\n", _userMediaPath.toUtf8( ).data( ) );

    QObject::connect( _usbRetryTimer, &QTimer::timeout, this, &SelectTab::usbRetryTimer_timeout );
    _usbRetryTimer->setInterval( 1000 );
    _usbRetryTimer->setSingleShot( false );
    _usbRetryTimer->setTimerType( Qt::PreciseTimer );

    _currentFsModel = _libraryFsModel;

    _libraryFsModel->setFilter( QDir::Files );
    _libraryFsModel->setNameFilterDisables( false );
    _libraryFsModel->setNameFilters( { { "*.stl" } } );
    _libraryFsModel->setRootPath( StlModelLibraryPath );
    QObject::connect( _libraryFsModel, &QFileSystemModel::directoryLoaded, this, &SelectTab::libraryFsModel_directoryLoaded );

    _usbFsModel->setFilter( QDir::Drives | QDir::Files );
    _usbFsModel->setNameFilterDisables( false );
    _usbFsModel->setNameFilters( { { "*.stl" } } );
    QObject::connect( _usbFsModel, &QFileSystemModel::directoryLoaded, this, &SelectTab::usbFsModel_directoryLoaded );

    QObject::connect( _fsWatcher, &QFileSystemWatcher::directoryChanged, this, &SelectTab::_lookForUsbStick );
    _fsWatcher->addPath( MediaRootPath );

    _lookForUsbStick( MediaRootPath );

    _availableFilesLabel->setText( "Models in library:" );

    _availableFilesListView->setFlow( QListView::TopToBottom );
    _availableFilesListView->setLayoutMode( QListView::SinglePass );
    _availableFilesListView->setMovement( QListView::Static );
    _availableFilesListView->setResizeMode( QListView::Fixed );
    _availableFilesListView->setViewMode( QListView::ListMode );
    _availableFilesListView->setWrapping( true );
    _availableFilesListView->setModel( _libraryFsModel );
    _availableFilesListView->grabGesture( Qt::SwipeGesture );
    QObject::connect( _availableFilesListView, &QListView::clicked, this, &SelectTab::availableFilesListView_clicked );

    _toggleLocationButton->setText( "Show USB stick" );
    QObject::connect( _toggleLocationButton, &QPushButton::clicked, this, &SelectTab::toggleLocationButton_clicked );

    _availableFilesLayout->setContentsMargins( { } );
    _availableFilesLayout->addWidget( _toggleLocationButton,   0, 0 );
    _availableFilesLayout->addWidget( _availableFilesLabel,    1, 0 );
    _availableFilesLayout->addWidget( _availableFilesListView, 2, 0 );

    _availableFilesContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _availableFilesContainer->setLayout( _availableFilesLayout );

    _selectButton->setEnabled( false );
    _selectButton->setFixedSize( MainButtonSize );
    _selectButton->setFont( ModifyFont( _selectButton->font( ), 22.0 ) );
    _selectButton->setText( "Select" );
    QObject::connect( _selectButton, &QPushButton::clicked, this, &SelectTab::selectButton_clicked );

    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
    format.setVersion( 2, 1 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( format );

    _canvas = new Canvas( format, this );
    _canvas->setMinimumWidth( MaximalRightHandPaneSize.width( ) );
    _canvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    _dimensionsErrorLabel->hide( );
    _dimensionsErrorLabel->setPalette( ModifyPalette( _dimensionsErrorLabel->palette( ), QPalette::WindowText, Qt::red ) );
    _dimensionsErrorLabel->setText( "Model exceeds build volume!" );

    _dimensionsLayout = WrapWidgetsInHBox( { _dimensionsLabel, nullptr, _dimensionsErrorLabel } );
    _dimensionsLayout->setAlignment( Qt::AlignLeft );
    _dimensionsLayout->setContentsMargins( { } );

    _canvasLayout->setContentsMargins( { } );
    _canvasLayout->addWidget( _canvas );
    _canvasLayout->addLayout( _dimensionsLayout );

    _layout->setContentsMargins( { } );
    _layout->addWidget( _availableFilesContainer, 0, 0, 1, 1 );
    _layout->addWidget( _selectButton,            1, 0, 1, 1 );
    _layout->addLayout( _canvasLayout,            0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

SelectTab::~SelectTab( ) {
    debug( "+ SelectTab::`dtor: destruct at %p\n", this );
}

void SelectTab::libraryFsModel_directoryLoaded( QString const& name ) {
    debug( "+ SelectTab::libraryFsModel_directoryLoaded: name '%s'\n", name.toUtf8( ).data( ) );
    if ( _modelsLocation == ModelsLocation::Library ) {
        _libraryFsModel->sort( 0, Qt::AscendingOrder );
        _availableFilesListView->setRootIndex( _libraryFsModel->index( StlModelLibraryPath ) );
    }
}

void SelectTab::usbFsModel_directoryLoaded( QString const& name ) {
    debug( "+ SelectTab::usbFsModel_directoryLoaded: name '%s'\n", name.toUtf8( ).data( ) );
    if ( _modelsLocation == ModelsLocation::Usb ) {
        _usbFsModel->sort( 0, Qt::AscendingOrder );
        _availableFilesListView->setRootIndex( _usbFsModel->index( _usbPath ) );
    }
}

void SelectTab::_lookForUsbStick( QString const& path ) {
    debug( "+ SelectTab::_lookForUsbStick: path '%s' has changed\n", path.toUtf8( ).data( ) );

    if ( path == MediaRootPath ) {
        if ( 0 != ::access( _userMediaPath.toUtf8( ).data( ), F_OK ) ) {
            error_t err = errno;
            debug( "  + access(F) failed: %s [%d]\n", strerror( err ), err );
            _fsWatcher->removePath( _userMediaPath );
            return;
        }
        _fsWatcher->addPath( _userMediaPath );
    }

    QString dirname { GetFirstDirectoryIn( _userMediaPath ) };
    if ( dirname.isEmpty( ) ) {
        debug( "  + no directories in user media path '%s'\n", _userMediaPath.toUtf8( ).data( ) );
        if ( _modelsLocation == ModelsLocation::Usb ) {
            _showLibrary( );
        }
        if ( _modelSelection.fileName.startsWith( _userMediaPath ) ) {
            _canvas->clear( );
            _dimensionsLabel->clear( );
            _selectButton->setEnabled( false );
        }
        _toggleLocationButton->setEnabled( false );
        return;
    }

    _usbPath = _userMediaPath + Slash + dirname;
    debug( "  + mounted USB device is '%s'\n", _usbPath.toUtf8( ).data( ) );

    struct stat statbuf { };
    if ( -1 == ::stat( _usbPath.toUtf8( ).data( ), &statbuf ) ) {
        error_t err = errno;
        debug( "  + stat failed: %s [%d]\n", strerror( err ), err );
    } else {
        debug( "  + stat reports mode 0%o, uid %d, gid %d\n", statbuf.st_mode, statbuf.st_uid, statbuf.st_gid );
        if ( ( statbuf.st_uid == 0 ) || ( statbuf.st_gid == 0 ) ) {
            if ( -1 == _usbRetryCount ) {
                debug( "  + waiting one second 3 more times\n" );
                _usbRetryCount = 3;
                _usbRetryTimer->start( );
            } else {
                _usbRetryCount--;
                if ( !_usbRetryCount ) {
                    debug( "  + done waiting, giving up\n" );
                    _usbRetryTimer->stop( );
                } else {
                    debug( "  + waiting one second %d more times\n", _usbRetryCount );
                }
            }
            return;
        }

        _usbRetryTimer->stop( );
    }

    if ( 0 != ::access( _usbPath.toUtf8( ).data( ), R_OK | X_OK ) ) {
        error_t err = errno;
        debug( "  + access(RX) failed: %s [%d]\n", strerror( err ), err );
        return;
    }

    _usbFsModel->setRootPath( _usbPath );
    _toggleLocationButton->setEnabled( true );
}

void SelectTab::availableFilesListView_clicked( QModelIndex const& index ) {
    int indexRow = index.row( );
    if ( _selectedRow == indexRow ) {
        return;
    }

    _modelSelection = { ( ( _modelsLocation == ModelsLocation::Library ) ? StlModelLibraryPath : _usbPath ) + Slash + index.data( ).toString( ), };
    _selectedRow    = indexRow;
    debug( "+ SelectTab::availableFilesListView_clicked: selection changed to row %d, file name '%s'\n", _selectedRow, _modelSelection.fileName.toUtf8( ).data( ) );

    _selectButton->setEnabled( false );
    _availableFilesListView->setEnabled( false );

    if ( _processRunner ) {
        QObject::disconnect( _processRunner, nullptr, this, nullptr );
        _processRunner->terminate( );
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }

    if ( !_loadModel( _modelSelection.fileName ) ) {
        debug( "  + _loadModel failed!\n" );
        _availableFilesListView->setEnabled( true );
    }
}

void SelectTab::availableFilesListView_swipeGesture( QGestureEvent* event, QSwipeGesture* gesture ) {
    debug(
        "+ SelectTab::availableFilesListView_swipeGesture:\n"
        "  + state:               %s [%d]\n"
        "  + horizontal direction %s [%d]\n"
        "  + vertical direction   %s [%d]\n"
        "",
        ToString( gesture->state( ) ),               gesture->state( ),
        ToString( gesture->horizontalDirection( ) ), gesture->horizontalDirection( ),
        ToString( gesture->verticalDirection( ) ),   gesture->verticalDirection( ),
        ToString( gesture->hasHotSpot( ) )
    );
    event->accept( );

    if ( !gesture->hasHotSpot( ) ) {
        debug( "  + gesture doesn't have a hotspot?\n" );
        return;
    }

    debug( "  + gesture hotspot:     %s\n", ToString( gesture->hotSpot( ) ) );
    switch ( gesture->state( ) ) {
        case Qt::GestureStarted:
            _swipeLastPoint = gesture->hotSpot( );
            break;

        case Qt::GestureUpdated:
            debug( "  + difference from previous position: %f\n", _swipeLastPoint.y( ) - gesture->hotSpot( ).y( ) );
            _swipeLastPoint = gesture->hotSpot( );
            break;

        case Qt::GestureFinished:
            debug( "  + difference from previous position: %f\n", _swipeLastPoint.y( ) - gesture->hotSpot( ).y( ) );
            _swipeLastPoint = gesture->hotSpot( );
            break;

        case Qt::GestureCanceled:
            break;

        case Qt::NoGesture:
            break;
    }
}

void SelectTab::toggleLocationButton_clicked( bool ) {
    if ( _modelsLocation == ModelsLocation::Library ) {
        _showUsbStick( );
    } else {
        _showLibrary( );
    }
}

void SelectTab::selectButton_clicked( bool ) {
    debug( "+ SelectTab::selectButton_clicked\n" );
    auto selection = new ModelSelectionInfo( _modelSelection );
    emit modelSelected( selection );
    delete selection;
}

void SelectTab::loader_gotMesh( Mesh* m ) {
    if ( _modelSelection.fileName.isEmpty( ) || ( _modelSelection.fileName[0].unicode( ) == L':' ) ) {
        debug( "+ SelectTab::loader_gotMesh: file name '%s' is empty or resource name\n", _modelSelection.fileName.toUtf8( ).data( ) );
        return;
    }

    _modelSelection.estimatedVolume = 0.0;
    m->bounds( _modelSelection.vertexCount, _modelSelection.x, _modelSelection.y, _modelSelection.z );

    debug(
        "+ SelectTab::loader_gotMesh:\n"
        "  + file name:         '%s'\n"
        "  + count of vertices: %5zu\n"
        "  + X range:           %12.6f .. %12.6f, %12.6f\n"
        "  + Y range:           %12.6f .. %12.6f, %12.6f\n"
        "  + Z range:           %12.6f .. %12.6f, %12.6f\n"
        "",
        _modelSelection.fileName.toUtf8( ).data( ),
        _modelSelection.vertexCount,
        _modelSelection.x.min, _modelSelection.x.max, _modelSelection.x.size,
        _modelSelection.y.min, _modelSelection.y.max, _modelSelection.y.size,
        _modelSelection.z.min, _modelSelection.z.max, _modelSelection.z.size
    );

    {
        auto sizeXstring = GroupDigits( QString( "%1" ).arg( _modelSelection.x.size, 0, 'f', 2 ), ' ' );
        auto sizeYstring = GroupDigits( QString( "%1" ).arg( _modelSelection.y.size, 0, 'f', 2 ), ' ' );
        auto sizeZstring = GroupDigits( QString( "%1" ).arg( _modelSelection.z.size, 0, 'f', 2 ), ' ' );
        _dimensionsLabel->setText( QString( "%1 mm × %2 mm × %3 mm" ).arg( sizeXstring ).arg( sizeYstring ).arg( sizeZstring ) );
    }

    _canvas->load_mesh( m );

    if ( ( _modelSelection.x.size > PrinterMaximumX ) || ( _modelSelection.y.size > PrinterMaximumY ) || ( _modelSelection.z.size > PrinterMaximumZ ) ) {
        _dimensionsErrorLabel->show( );
        _selectButton->setEnabled( false );
        emit modelSelectionFailed( );
        return;
    } else {
        _dimensionsErrorLabel->hide( );
    }

    if ( _processRunner ) {
        QObject::disconnect( _processRunner, nullptr, this, nullptr );
        _processRunner->terminate( );
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }

    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &SelectTab::processRunner_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &SelectTab::processRunner_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &SelectTab::processRunner_readyReadStandardOutput );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  this, &SelectTab::processRunner_readyReadStandardError  );

    _processRunner->start(
        { "slic3r" },
        {
            { "--info"                 },
            { _modelSelection.fileName }
        }
    );
}

void SelectTab::loader_ErrorBadStl( ) {
    debug( "+ SelectTab::loader_ErrorBadStl\n" );
    QMessageBox::critical( this, "Error",
        "<b>Error:</b><br>"
        "This <code>.stl</code> file is invalid or corrupted.<br>"
        "Please export it from the original source, verify, and retry."
    );
    emit modelSelectionFailed( );
}

void SelectTab::loader_ErrorEmptyMesh( ) {
    debug( "+ SelectTab::loader_ErrorEmptyMesh\n" );
    QMessageBox::critical( this, "Error",
        "<b>Error:</b><br>"
        "This file is syntactically correct<br>but contains no triangles."
    );
    emit modelSelectionFailed( );
}

void SelectTab::loader_ErrorMissingFile( ) {
    debug( "+ SelectTab::loader_ErrorMissingFile\n" );
    QMessageBox::critical( this, "Error",
        "<b>Error:</b><br>"
        "The target file is missing.<br>"
    );
    emit modelSelectionFailed( );
}

void SelectTab::loader_Finished( ) {
    debug( "+ SelectTab::loader_Finished\n" );
    _availableFilesListView->setEnabled( true );
    _canvas->clear_status( );
    _loader->deleteLater( );
    _loader = nullptr;
}

void SelectTab::processRunner_succeeded( ) {
    debug( "+ SelectTab::processRunner_succeeded\n" );

    bool gotVolume = false;
    for ( auto line : _slicerBuffer.split( QRegularExpression { QString { "\\r?\\n" } } ) ) {
        auto match = VolumeLineMatcher.match( line );
        if ( match.hasMatch( ) ) {
            _modelSelection.estimatedVolume = match.captured( 1 ).toDouble( );
            _dimensionsLabel->setText( _dimensionsLabel->text( ) + QString( "  •  %1 mL" ).arg( GroupDigits( QString( "%1" ).arg( _modelSelection.estimatedVolume, 0, 'f', 2 ), ' ' ) ) );
            gotVolume = true;
            break;
        }
    }

    if ( gotVolume ) {
        _selectButton->setEnabled( true );
    }

    _slicerBuffer.clear( );
}

void SelectTab::processRunner_failed( QProcess::ProcessError const error ) {
    debug( "SelectTab::processRunner_failed: error %s [%d]\n", ToString( error ), error );
    _slicerBuffer.clear( );
    emit modelSelectionFailed( );
}

void SelectTab::processRunner_readyReadStandardOutput( QString const& data ) {
    debug( "+ SelectTab::processRunner_readyReadStandardOutput: %d bytes from slic3r\n", data.length( ) );
    _slicerBuffer += data;
}

void SelectTab::processRunner_readyReadStandardError( QString const& data ) {
    debug(
        "+ SelectTab::processRunner_readyReadStandardError: %d bytes from slic3r:\n"
        "%s",
        data.length( ),
        data.toUtf8( ).data( )
    );
}

void SelectTab::usbRetryTimer_timeout( ) {
    _lookForUsbStick( _userMediaPath );
}

bool SelectTab::_loadModel( QString const& fileName ) {
    debug( "+ SelectTab::_loadModel: fileName: '%s'\n", fileName.toUtf8( ).data( ) );
    if ( _loader ) {
        debug( "+ SelectTab::_loadModel: loader object exists, not loading\n" );
        return false;
    }

    _canvas->set_status( QString( "Loading " ) + GetFileBaseName( fileName ) );

    _loader = new Loader( fileName, this );
    connect( _loader, &Loader::got_mesh,           this,    &SelectTab::loader_gotMesh          );
    connect( _loader, &Loader::error_bad_stl,      this,    &SelectTab::loader_ErrorBadStl      );
    connect( _loader, &Loader::error_empty_mesh,   this,    &SelectTab::loader_ErrorEmptyMesh   );
    connect( _loader, &Loader::error_missing_file, this,    &SelectTab::loader_ErrorMissingFile );
    connect( _loader, &Loader::finished,           this,    &SelectTab::loader_Finished         );

    _selectButton->setEnabled( false );
    _loader->start( );
    return true;
}

void SelectTab::_showLibrary( ) {
    _modelsLocation = ModelsLocation::Library;
    _currentFsModel = _libraryFsModel;

    _libraryFsModel->sort( 0, Qt::AscendingOrder );
    _availableFilesLabel->setText( "Models in library:" );
    _availableFilesListView->setModel( _libraryFsModel );
    _availableFilesListView->setRootIndex( _libraryFsModel->index( StlModelLibraryPath ) );
    _toggleLocationButton->setText( "Show USB stick" );
}

void SelectTab::_showUsbStick( ) {
    _modelsLocation = ModelsLocation::Usb;
    _currentFsModel = _usbFsModel;

    _usbFsModel->sort( 0, Qt::AscendingOrder );
    _availableFilesLabel->setText( "Models on USB stick:" );
    _availableFilesListView->setModel( _usbFsModel );
    _availableFilesListView->setRootIndex( _usbFsModel->index( _usbPath ) );
    _toggleLocationButton->setText( "Show library" );
}

void SelectTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ StatusTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void SelectTab::setShepherd( Shepherd* newShepherd ) {
    if ( _shepherd ) {
        QObject::disconnect( _shepherd, nullptr, this, nullptr );
    }

    _shepherd = newShepherd;
}
