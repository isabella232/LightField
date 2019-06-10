#include "pch.h"

#include "filetab.h"

#include "app.h"
#include "canvas.h"
#include "loader.h"
#include "mesh.h"
#include "printjob.h"
#include "processrunner.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"
#include "upgrademanager.h"

namespace {

    QRegularExpression VolumeLineMatcher { QString { "^\\s*volume\\s*[:=]\\s*(\\d+(?:\\.(?:\\d+))?)" }, QRegularExpression::CaseInsensitiveOption };

}

FileTab::FileTab( QWidget* parent ): InitialShowEventMixin<FileTab, TabBase>( parent ) {
    debug(
        "+ FileTab::`ctor:\n"
        "  + Model library directory: '%s'\n"
        "",
        StlModelLibraryPath.toUtf8( ).data( )
    );

    _currentFsModel = _libraryFsModel;

    _libraryFsModel->setFilter( QDir::Files );
    _libraryFsModel->setNameFilterDisables( false );
    _libraryFsModel->setNameFilters( { { "*.stl" } } );
    _libraryFsModel->setRootPath( StlModelLibraryPath );
    QObject::connect( _libraryFsModel, &QFileSystemModel::directoryLoaded, this, &FileTab::libraryFsModel_directoryLoaded );

    _usbFsModel->setFilter( QDir::Files );
    _usbFsModel->setNameFilterDisables( false );
    _usbFsModel->setNameFilters( { { "*.stl" } } );
    QObject::connect( _usbFsModel, &QFileSystemModel::directoryLoaded, this, &FileTab::usbFsModel_directoryLoaded );

    _toggleLocationButton->setEnabled( false );
    _toggleLocationButton->setText( "Show USB stick" );
    QObject::connect( _toggleLocationButton, &QPushButton::clicked, this, &FileTab::toggleLocationButton_clicked );

    _availableFilesLabel->setText( "Models in library:" );

    _availableFilesListView->setFlow( QListView::TopToBottom );
    _availableFilesListView->setLayoutMode( QListView::SinglePass );
    _availableFilesListView->setMovement( QListView::Static );
    _availableFilesListView->setResizeMode( QListView::Fixed );
    _availableFilesListView->setViewMode( QListView::ListMode );
    _availableFilesListView->setModel( _libraryFsModel );
    _availableFilesListView->grabGesture( Qt::SwipeGesture );
    QObject::connect( _availableFilesListView, &QListView::clicked, this, &FileTab::availableFilesListView_clicked );

    _availableFilesLayout->setContentsMargins( { } );
    _availableFilesLayout->addWidget( _toggleLocationButton,   0, 0 );
    _availableFilesLayout->addWidget( _availableFilesLabel,    1, 0 );
    _availableFilesLayout->addWidget( _availableFilesListView, 2, 0 );

    _availableFilesContainer->setFixedWidth( MainButtonSize.width( ) );
    _availableFilesContainer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _availableFilesContainer->setLayout( _availableFilesLayout );

    _selectButton->setEnabled( false );
    _selectButton->setFixedSize( MainButtonSize );
    _selectButton->setFont( ModifyFont( _selectButton->font( ), 22.0 ) );
    _selectButton->setText( "Select" );
    QObject::connect( _selectButton, &QPushButton::clicked, this, &FileTab::selectButton_clicked );

    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
    format.setVersion( 2, 1 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( format );

    _canvas = new Canvas( format, this );
    _canvas->setMinimumWidth( MaximalRightHandPaneSize.width( ) );
    _canvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    _errorLabel->setAlignment( Qt::AlignRight );
    _errorLabel->setPalette( ModifyPalette( _errorLabel->palette( ), QPalette::WindowText, Qt::red ) );

    _dimensionsLabel->setTextFormat( Qt::RichText );

    _dimensionsLayout = WrapWidgetsInHBox( { _dimensionsLabel, nullptr, _errorLabel } );
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

FileTab::~FileTab( ) {
    /*empty*/
}

void FileTab::_initialShowEvent( QShowEvent* event ) {
    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );

    event->accept( );
}

void FileTab::_loadModel( QString const& fileName ) {
    debug( "+ FileTab::_loadModel: fileName: '%s'\n", fileName.toUtf8( ).data( ) );
    _canvas->set_status( QString( "Loading " ) + GetFileBaseName( fileName ) );
    update( );

    if ( _loader ) {
        QObject::disconnect( _loader, nullptr, this, nullptr );
        _loader->deleteLater( );
    }
    _loader = new Loader( fileName, this );

    QObject::connect( _loader, &Loader::got_mesh,           this, &FileTab::loader_gotMesh          );
    QObject::connect( _loader, &Loader::error_bad_stl,      this, &FileTab::loader_errorBadStl      );
    QObject::connect( _loader, &Loader::error_empty_mesh,   this, &FileTab::loader_errorEmptyMesh   );
    QObject::connect( _loader, &Loader::error_missing_file, this, &FileTab::loader_errorMissingFile );
    QObject::connect( _loader, &Loader::finished,           this, &FileTab::loader_finished         );

    _loader->start( );
}

void FileTab::_showLibrary( ) {
    _modelsLocation = ModelsLocation::Library;
    _currentFsModel = _libraryFsModel;

    _libraryFsModel->sort( 0, Qt::AscendingOrder );
    _availableFilesLabel->setText( "Models in library:" );
    _availableFilesListView->setModel( _libraryFsModel );
    _availableFilesListView->setRootIndex( _libraryFsModel->index( StlModelLibraryPath ) );
    _toggleLocationButton->setText( "Show USB stick" );

    update( );
}

void FileTab::_showUsbStick( ) {
    _modelsLocation = ModelsLocation::Usb;
    _currentFsModel = _usbFsModel;

    _usbFsModel->sort( 0, Qt::AscendingOrder );
    _availableFilesLabel->setText( "Models on USB stick:" );
    _availableFilesListView->setModel( _usbFsModel );
    _availableFilesListView->setRootIndex( _usbFsModel->index( _usbPath ) );
    _toggleLocationButton->setText( "Show library" );

    update( );
}

void FileTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ FileTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;
    switch ( _uiState ) {
        case UiState::SelectStarted:
            _modelSelection = { };
            break;

        case UiState::SelectCompleted:
        case UiState::SliceStarted:
        case UiState::SliceCompleted:
        case UiState::PrintStarted:
        case UiState::PrintCompleted:
            break;
    }
}

void FileTab::usbMountManager_filesystemMounted( QString const& mountPoint ) {
    debug( "+ FileTab::usbMountManager_filesystemMounted: mount point %s\n", mountPoint.toUtf8( ).data( ) );

    if ( !_usbPath.isEmpty( ) ) {
        debug( "  + USB storage device at %s already mounted; ignoring new mount\n", _usbPath.toUtf8( ).data( ) );
        return;
    }

    QFileInfo usbPathInfo { mountPoint };
    if ( !usbPathInfo.isReadable( ) || !usbPathInfo.isExecutable( ) ) {
        debug( "  + USB path is inaccessible (uid: %u; gid: %u; mode: %04o)\n", usbPathInfo.ownerId( ), usbPathInfo.groupId( ), usbPathInfo.permissions( ) & 07777 );
        return;
    }

    _usbPath = mountPoint;
    _usbFsModel->setRootPath( mountPoint );
    _toggleLocationButton->setEnabled( true );

    update( );
}

void FileTab::usbMountManager_filesystemUnmounted( QString const& mountPoint ) {
    debug( "+ FileTab::usbMountManager_filesystemUnmounted: mount point %s\n", mountPoint.toUtf8( ).data( ) );

    if ( mountPoint != _usbPath ) {
        debug( "  + not our filesystem; ignoring\n", _usbPath.toUtf8( ).data( ) );
        return;
    }

    if ( _modelsLocation == ModelsLocation::Usb ) {
        _showLibrary( );
    }
    if ( _modelSelection.fileName.startsWith( _usbPath ) ) {
        _canvas->clear( );
        _dimensionsLabel->clear( );
        _errorLabel->clear( );

        emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
    }

    _usbPath.clear( );
    _toggleLocationButton->setEnabled( false );

    update( );
}

void FileTab::loader_gotMesh( Mesh* mesh ) {
    if ( _modelSelection.fileName.isEmpty( ) || ( _modelSelection.fileName[0] == L':' ) ) {
        _dimensionsLabel->clear( );
        _errorLabel->clear( );
        update( );

        delete mesh;
        return;
    }

    _modelSelection.estimatedVolume = 0.0;
    mesh->bounds( _modelSelection.vertexCount, _modelSelection.x, _modelSelection.y, _modelSelection.z );

    debug(
        "+ FileTab::loader_gotMesh:\n"
        "  + count of vertices: %9zu\n"
        "  + X range:               %12.6f .. %12.6f, %12.6f\n"
        "  + Y range:               %12.6f .. %12.6f, %12.6f\n"
        "  + Z range:               %12.6f .. %12.6f, %12.6f\n"
        "",
        _modelSelection.fileName.toUtf8( ).data( ),
        _modelSelection.vertexCount,
        _modelSelection.x.min, _modelSelection.x.max, _modelSelection.x.size,
        _modelSelection.y.min, _modelSelection.y.max, _modelSelection.y.size,
        _modelSelection.z.min, _modelSelection.z.max, _modelSelection.z.size
    );

    _dimensionsText = QString { "%1 mm × %2 mm × %3 mm" }
        .arg( GroupDigits( QString { "%1" }.arg( _modelSelection.x.size, 0, 'f', 2 ), ' ' ) )
        .arg( GroupDigits( QString { "%1" }.arg( _modelSelection.y.size, 0, 'f', 2 ), ' ' ) )
        .arg( GroupDigits( QString { "%1" }.arg( _modelSelection.z.size, 0, 'f', 2 ), ' ' ) );
    _dimensionsLabel->setText( _dimensionsText + Space + BlackDiamond + QString { " <i>calculating volume...</i>" } );

    _canvas->load_mesh( mesh );

    if ( ( _modelSelection.x.size > PrinterMaximumX ) || ( _modelSelection.y.size > PrinterMaximumY ) || ( _modelSelection.z.size > PrinterMaximumZ ) ) {
        _dimensionsLabel->setText( _dimensionsText );
        _errorLabel->setText( "Model exceeds build volume!" );

        emit uiStateChanged( TabIndex::File, UiState::SelectStarted );

        update( );
        return;
    } else {
        _errorLabel->clear( );
        update( );
    }

    if ( _processRunner ) {
        QObject::disconnect( _processRunner, nullptr, this, nullptr );
        _processRunner->terminate( );
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }

    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &FileTab::processRunner_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &FileTab::processRunner_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &FileTab::processRunner_readyReadStandardOutput );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  this, &FileTab::processRunner_readyReadStandardError  );

    _processRunner->start(
        { "slic3r" },
        {
            "--info", _modelSelection.fileName,
        }
    );
}

void FileTab::loader_errorBadStl( ) {
    debug( "+ FileTab::loader_errorBadStl\n" );

    _dimensionsLabel->clear( );
    _errorLabel->setText( "Unable to read model." );
    update( );

    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::loader_errorEmptyMesh( ) {
    debug( "+ FileTab::loader_errorEmptyMesh\n" );

    _dimensionsLabel->clear( );
    _errorLabel->setText( "Model is empty" );
    update( );

    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::loader_errorMissingFile( ) {
    debug( "+ FileTab::loader_errorMissingFile\n" );

    _dimensionsLabel->clear( );
    _errorLabel->setText( "Model file went missing" );
    update( );

    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::loader_finished( ) {
    debug( "+ FileTab::loader_finished\n" );

    _availableFilesListView->setEnabled( true );
    _canvas->clear_status( );
    update( );

    _loader->deleteLater( );
    _loader = nullptr;
}

void FileTab::libraryFsModel_directoryLoaded( QString const& name ) {
    debug( "+ FileTab::libraryFsModel_directoryLoaded\n" );
    if ( _modelsLocation == ModelsLocation::Library ) {
        _libraryFsModel->sort( 0, Qt::AscendingOrder );
        _availableFilesListView->setRootIndex( _libraryFsModel->index( StlModelLibraryPath ) );
        update( );
    }
}

void FileTab::usbFsModel_directoryLoaded( QString const& name ) {
    debug( "+ FileTab::usbFsModel_directoryLoaded\n" );
    if ( _modelsLocation == ModelsLocation::Usb ) {
        _usbFsModel->sort( 0, Qt::AscendingOrder );
        _availableFilesListView->setRootIndex( _usbFsModel->index( _usbPath ) );
        update( );
    }
}

void FileTab::availableFilesListView_clicked( QModelIndex const& index ) {
    int indexRow = index.row( );
    if ( _selectedRow == indexRow ) {
        return;
    }

    _modelSelection = { ( ( _modelsLocation == ModelsLocation::Library ) ? StlModelLibraryPath : _usbPath ) + Slash + index.data( ).toString( ) };
    _selectedRow    = indexRow;

    _selectButton->setEnabled( false );
    _availableFilesListView->setEnabled( false );
    update( );

    if ( _processRunner ) {
        QObject::disconnect( _processRunner, nullptr, this, nullptr );
        _processRunner->terminate( );
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }

    _loadModel( _modelSelection.fileName );
}

void FileTab::availableFilesListView_swipeGesture( QGestureEvent* event, QSwipeGesture* gesture ) {
    debug(
        "+ FileTab::availableFilesListView_swipeGesture:\n"
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

void FileTab::toggleLocationButton_clicked( bool ) {
    if ( _modelsLocation == ModelsLocation::Library ) {
        _showUsbStick( );
    } else {
        _showLibrary( );
    }

    update( );
}

void FileTab::selectButton_clicked( bool ) {
    debug( "+ FileTab::selectButton_clicked\n" );
    emit modelSelected( &_modelSelection );
    emit uiStateChanged( TabIndex::File, UiState::SelectCompleted );

    update( );
}

void FileTab::processRunner_succeeded( ) {
    debug( "+ FileTab::processRunner_succeeded\n" );

    for ( auto line : _slicerBuffer.split( NewLineRegex ) ) {
        auto match = VolumeLineMatcher.match( line );
        if ( match.hasMatch( ) ) {
            _modelSelection.estimatedVolume = match.captured( 1 ).toDouble( );

            QString unit;
            if ( _modelSelection.estimatedVolume < 1000.0 ) {
                unit = "µL";
            } else {
                _modelSelection.estimatedVolume /= 1000.0;
                unit = "mL";
            }
            _dimensionsLabel->setText(
                _dimensionsText + Space +
                BlackDiamond + Space +
                GroupDigits( QString{ "%1" }.arg( _modelSelection.estimatedVolume, 0, 'f', 2 ), ' ' ) + Space +
                unit
            );
            _selectButton->setEnabled( true );

            update( );
            break;
        }
    }

    _slicerBuffer.clear( );
}

void FileTab::processRunner_failed( int const exitCode, QProcess::ProcessError const error ) {
    debug( "FileTab::processRunner_failed: exit code: %d, error %s [%d]\n", exitCode, ToString( error ), error );

    _slicerBuffer.clear( );
    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::processRunner_readyReadStandardOutput( QString const& data ) {
    debug( "+ FileTab::processRunner_readyReadStandardOutput: %d bytes from slic3r\n", data.length( ) );
    _slicerBuffer += data;
}

void FileTab::processRunner_readyReadStandardError( QString const& data ) {
    auto tmp = data.toUtf8( );
    fwrite( tmp.data( ), 1, tmp.count( ), stderr );
}
