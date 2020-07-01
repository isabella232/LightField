#include "pch.h"

#include <sys/sysinfo.h>

#include "filetab.h"

#include "app.h"
#include "canvas.h"
#include "filecopier.h"
#include "loader.h"
#include "mesh.h"
#include "printjob.h"
#include "printmanager.h"
#include "processrunner.h"
#include "shepherd.h"
#include "timinglogger.h"
#include "usbmountmanager.h"
#include "window.h"

namespace {

    QRegularExpression VolumeLineMatcher { QString { "^\\s*volume\\s*[:=]\\s*(\\d+(?:\\.(?:\\d+))?)" }, QRegularExpression::CaseInsensitiveOption };

    QString ModelFileNameToDelete;

    char const* ModelsLocationStrings[] {
        "Library",
        "Usb",
    };

    char const* ModelFileTypeStrings[] {
        "File",
        "Directory"
    };

    char const* ToString( ModelsLocation const value ) {
#if defined _DEBUG
        if ( ( value >= ModelsLocation::Library ) && ( value <= ModelsLocation::Usb ) ) {
#endif
            return ModelsLocationStrings[static_cast<int>( value )];
#if defined _DEBUG
        } else {
            return nullptr;
        }
#endif
    }

    char const* ToString( ModelFileType const value ) {
#if defined _DEBUG
        if ( ( value >= ModelFileType::File ) && ( value <= ModelFileType::Directory ) ) {
#endif
            return ModelFileTypeStrings[static_cast<int>( value )];
#if defined _DEBUG
        } else {
            return nullptr;
        }
#endif
    }

}

FileTab::FileTab( QWidget* parent ): InitialShowEventMixin<FileTab, TabBase>( parent ) {
    QFont font16pt { ModifyFont( font( ), 16.0          ) };
    QFont font22pt { ModifyFont( font( ), LargeFontSize ) };

    _libraryFsModel->setFilter( QDir::Files | QDir::Dirs );
    _libraryFsModel->setNameFilterDisables( false );
    _libraryFsModel->setNameFilters( {
        { "*.stl" },
#if defined EXPERIMENTAL
        { "*-20"  },
#endif
        { "*-50"  },
        { "*-100" },
        { "*tiled*" }
    } );
    _libraryFsModel->setRootPath( StlModelLibraryPath );
    QObject::connect( _libraryFsModel, &QFileSystemModel::directoryLoaded, this, &FileTab::libraryFsModel_directoryLoaded );

    _toggleLocationButton->setEnabled( false );
    _toggleLocationButton->setFont( font16pt );
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

    _selectButton->setEnabled( false );
    _selectButton->setFixedSize( MainButtonSize );
    _selectButton->setFont( font22pt );
    _selectButton->setText( "Select" );
    QObject::connect( _selectButton, &QPushButton::clicked, this, &FileTab::selectButton_clicked );

    _leftColumn->setContentsMargins( { } );
    _leftColumn->setFixedWidth( MainButtonSize.width( ) );
    _leftColumn->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _leftColumn->setLayout( WrapWidgetsInVBox(
        _toggleLocationButton,
        _availableFilesLabel,
        _availableFilesListView,
        _selectButton
    ) );


    _canvas = new Canvas( this );
    _canvas->setFixedSize( 735, 490 );
    _canvas->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _dimensionsLabel->setAlignment( Qt::AlignLeft | Qt::AlignBottom );
    _dimensionsLabel->setTextFormat( Qt::RichText );
    _dimensionsLabel->setText( "Loading... " );

    _errorLabel->setAlignment( Qt::AlignRight | Qt::AlignBottom );
    _errorLabel->setTextFormat( Qt::RichText );

    _deleteButton = new QPushButton( "Delete file", this );
    _deleteButton->setFont( font16pt );
    _deleteButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _deleteButton->setFixedSize( SmallMainButtonSize );
    QObject::connect( _deleteButton, &QPushButton::clicked, this, &FileTab::deleteButton_clicked );

    _viewSolid->setChecked( true );
    _viewSolid->setEnabled( false );
    _viewSolid->setFont( font16pt );
    _viewSolid->setText( "Solid" );
    QObject::connect( _viewSolid, &QRadioButton::toggled, this, &FileTab::viewSolid_toggled );

    _viewWireframe->setChecked( false );
    _viewWireframe->setEnabled( false );
    _viewWireframe->setFont( font16pt );
    _viewWireframe->setText( "Wireframe" );
    QObject::connect( _viewWireframe, &QRadioButton::toggled, this, &FileTab::viewWireframe_toggled );

    auto viewButtonsLayout = WrapWidgetsInHBox( nullptr, _viewSolid, _viewWireframe );
    viewButtonsLayout->setAlignment( Qt::AlignVCenter );

    _rightColumn->setContentsMargins( { } );
    _rightColumn->setMinimumSize( MaximalRightHandPaneSize );
    _rightColumn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _rightColumn->setLayout( WrapWidgetsInVBox(
        viewButtonsLayout,
        _canvas,
        WrapWidgetsInHBox(nullptr, _deleteButton),
        WrapWidgetsInHBox( _dimensionsLabel, nullptr, _errorLabel )
    ) );


    setLayout( WrapWidgetsInHBox( _leftColumn, _rightColumn ) );

    update();
}

FileTab::~FileTab( ) {
    /*empty*/
}

void FileTab::_initialShowEvent( QShowEvent* event ) {
    _deleteButton->setEnabled( false );
    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );

    _dimensionsLabel->clear();
    _errorLabel->clear( );

    event->accept( );
}

void FileTab::_connectPrintManager()
{

    if (_printManager) {
        QObject::connect(_printManager, &PrintManager::printStarting, this,
           &FileTab::printManager_printStarting);
        QObject::connect(_printManager, &PrintManager::printComplete, this,
           &FileTab::printManager_printComplete);
        QObject::connect(_printManager, &PrintManager::printAborted, this,
           &FileTab::printManager_printAborted);
    }
}

void FileTab::printManager_printStarting()
{

    debug("+ FileTab::printManager_printStarting\n");

    _selectButton->setEnabled(false);
    _deleteButton->setEnabled(false);

    update();
}

void FileTab::printManager_printComplete(const bool success)
{

    debug("+ FileTab::printManager_printComplete\n");
    (void)success;

    _selectButton->setEnabled(true);
    _deleteButton->setEnabled(true);

    update();
}

void FileTab::printManager_printAborted()
{

    debug("+ FileTab::printManager_printAborted\n");

    _selectButton->setEnabled(true);
    _deleteButton->setEnabled(true);

    update();
}

void FileTab::_connectUsbMountManager( ) {
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemMounted,   this, &FileTab::usbMountManager_filesystemMounted   );
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemUnmounted, this, &FileTab::usbMountManager_filesystemUnmounted );
}

void FileTab::_createUsbFsModel( ) {
    _destroyUsbFsModel( );

    _usbFsModel = new QFileSystemModel;
    _usbFsModel->setFilter( QDir::Files | QDir::Dirs );
    _usbFsModel->setNameFilterDisables( false );
    _usbFsModel->setNameFilters( {
        { "*.stl" },
#if defined EXPERIMENTAL
        { "*-20"  },
#endif
        { "*-50"  },
        { "*-100" },
        { "*tiled*" }
    } );
    (void) QObject::connect( _usbFsModel, &QFileSystemModel::directoryLoaded, this, &FileTab::usbFsModel_directoryLoaded );
    _usbFsModel->setRootPath( _usbPath );
}

void FileTab::_destroyUsbFsModel( ) {
    if ( _usbFsModel ) {
        QObject::disconnect( _usbFsModel );
        _usbFsModel->deleteLater( );
        _usbFsModel = nullptr;
    }
}

void FileTab::_loadModel( QString const& fileName ) {
    debug( "+ FileTab::_loadModel: fileName: '%s'\n", fileName.toUtf8( ).data( ) );
    _dimensionsLabel->setText( "Loading " % GetFileBaseName( fileName ) );
    _errorLabel->clear( );
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

void FileTab::_deleteModel( ) {
    if ( !ModelFileNameToDelete.isEmpty( ) ) {
        debug( "+ FileTab::_deleteModel: attempting to delete file '%s'\n", ModelFileNameToDelete.toUtf8( ).data( ) );
        if ( -1 == unlink( ModelFileNameToDelete.toUtf8( ).data( ) ) ) {
            error_t err = errno;
            debug( "  + failed to delete file: %s [%d]\n", strerror( err ), err );
            return;
        }
        ModelFileNameToDelete.clear( );
    }

    _clearSelection( );
    update( );
}

void FileTab::_clearSelection( ) {
    _modelSelection = { };
    _selectedRow    = -1;

    _availableFilesListView->selectionModel( )->clear( );
    _selectButton->setEnabled( false );
    _dimensionsLabel->clear( );
    _errorLabel->clear( );
    _viewSolid->setEnabled( false );
    _viewWireframe->setEnabled( false );
    _deleteButton->setEnabled( false );

    QTimer::singleShot( 1, [this] ( ) { _canvas->clear( ); } );
}

void FileTab::_showLibrary( ) {
    _clearSelection( );
    _modelsLocation = ModelsLocation::Library;

    _libraryFsModel->sort( 0, Qt::AscendingOrder );

    _toggleLocationButton->setText( "Show USB stick" );
    _availableFilesLabel->setText( "Models in library:" );
    _availableFilesListView->setEnabled( true );
    _availableFilesListView->setModel( _libraryFsModel );
    _availableFilesListView->setRootIndex( _libraryFsModel->index( StlModelLibraryPath ) );
    _selectButton->setText( "Select" );

    update( );
}

void FileTab::_showUsbStick( ) {
    _clearSelection( );
    _modelsLocation = ModelsLocation::Usb;

    _usbFsModel->sort( 0, Qt::AscendingOrder );

    _toggleLocationButton->setText( "Show library" );
    _availableFilesLabel->setText( "Models on USB stick:" );
    _availableFilesListView->setEnabled( true );
    _availableFilesListView->setModel( _usbFsModel );
    _availableFilesListView->setRootIndex( _usbFsModel->index( _usbPath ) );
    _selectButton->setText( "Copy to library" );

    update( );
}

void FileTab::tab_uiStateChanged( TabIndex const sender, UiState const state )
{
    debug( "+ FileTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;
    switch (_uiState) {
    case UiState::SelectStarted:
        _selectedRow = -1;
        break;

    case UiState::SelectCompleted:
        if (sender == TabIndex::Tiling) {
            auto index = _libraryFsModel->index( _printJob->getLayerDirectory(0) );
            _availableFilesListView->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Select );
        }
        break;

    default:
        break;
    }
}

void FileTab::usbMountManager_filesystemMounted( QString const& mountPoint ) {
    debug( "+ FileTab::usbMountManager_filesystemMounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );

    if ( !_usbPath.isEmpty( ) ) {
        debug( "  + We already have a USB storage device at '%s' mounted; ignoring new mount\n", _usbPath.toUtf8( ).data( ) );
        return;
    }

    QFileInfo usbPathInfo { mountPoint };
    if ( !usbPathInfo.isReadable( ) || !usbPathInfo.isExecutable( ) ) {
        debug( "  + Unable to access mount point '%s' (uid: %u; gid: %u; mode: 0%03o)\n", _usbPath.toUtf8( ).data( ), usbPathInfo.ownerId( ), usbPathInfo.groupId( ), usbPathInfo.permissions( ) & 07777 );
        return;
    }

    _usbPath = mountPoint;
    _createUsbFsModel( );
    _toggleLocationButton->setEnabled( true );

    update( );
}

void FileTab::usbMountManager_filesystemRemounted( bool const succeeded, bool const writable ) {
    debug( "+ FileTab::usbMountManager_filesystemRemounted: succeeded? %s; writable? %s\n", YesNoString( succeeded ), YesNoString( writable ) );
    QObject::disconnect( _usbMountManager, &UsbMountManager::filesystemRemounted, this, &FileTab::usbMountManager_filesystemRemounted );

    if ( succeeded && writable ) {
        debug( "+ FileTab::usbMountManager_filesystemRemounted: deleting model\n" );
        _deleteModel( );
        _usbMountManager->remount( false );
    }

    App::mainWindow( )->show( );
}

void FileTab::usbMountManager_filesystemUnmounted( QString const& mountPoint ) {
    debug( "+ FileTab::usbMountManager_filesystemUnmounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );

    if ( mountPoint != _usbPath ) {
        debug( "  + not our filesystem; ignoring\n", _usbPath.toUtf8( ).data( ) );
        return;
    }

    if ( _modelsLocation == ModelsLocation::Usb ) {
        _showLibrary( );
    }

    if ( ( _selectedRow != -1 ) && _modelSelection.fileName.startsWith( _usbPath ) ) {
        _canvas->clear( );
        _dimensionsLabel->clear( );
        _errorLabel->clear( );

        emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
    }

    _usbPath.clear( );
    _destroyUsbFsModel( );
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
        _modelSelection.vertexCount,
        _modelSelection.x.min, _modelSelection.x.max, _modelSelection.x.size,
        _modelSelection.y.min, _modelSelection.y.max, _modelSelection.y.size,
        _modelSelection.z.min, _modelSelection.z.max, _modelSelection.z.size
    );

    debug(
        "Printer Maximum X: %12.6f\n"
        "Printer Maximum Y: %12.6f\n"
        "Printer Maximum Z: %12.6f\n"
        "",
        PrinterMaximumX,
        PrinterMaximumY,
        PrinterMaximumZ
    );

    _dimensionsText = QString { "%1 mm × %2 mm × %3 mm" }
        .arg( GroupDigits( QString { "%1" }.arg( _modelSelection.x.size, 0, 'f', 2 ), ' ' ) )
        .arg( GroupDigits( QString { "%1" }.arg( _modelSelection.y.size, 0, 'f', 2 ), ' ' ) )
        .arg( GroupDigits( QString { "%1" }.arg( _modelSelection.z.size, 0, 'f', 2 ), ' ' ) );
    _dimensionsLabel->setText( _dimensionsText % ", <i>calculating volume…</i>" );

    if ( _viewSolid->isChecked( ) ) {
        _canvas->draw_shaded( );
    } else {
        _canvas->draw_wireframe( );
    }
    _canvas->load_mesh( mesh );

    if ( ( _modelSelection.x.size > PrinterMaximumX ) || ( _modelSelection.y.size > PrinterMaximumY ) || ( _modelSelection.z.size > PrinterMaximumZ ) ) {
        _dimensionsLabel->setText( _dimensionsText );
        _errorLabel->setText( "<span style=\"color: red;\">Model exceeds build volume!</span>" );
        _deleteButton->setEnabled( true );

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

    TimingLogger::startTiming( TimingId::VolumeCalculation, GetFileBaseName( _modelSelection.fileName ) );

    _processRunner->start(
        { "slic3r" },
        {
            "--threads", QString { "%1" }.arg( get_nprocs( ) ),
            "--info",    _modelSelection.fileName,
        }
    );
}

void FileTab::loader_errorBadStl( ) {
    debug( "+ FileTab::loader_errorBadStl\n" );

    _dimensionsLabel->clear( );
    _errorLabel->setText( "<span style=\"color: red;\">Unable to read model</span>" );
    update( );

    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::loader_errorEmptyMesh( ) {
    debug( "+ FileTab::loader_errorEmptyMesh\n" );

    _dimensionsLabel->clear( );
    _errorLabel->setText( "<span style=\"color: red;\">Model is empty</span>" );
    update( );

    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::loader_errorMissingFile( ) {
    debug( "+ FileTab::loader_errorMissingFile\n" );

    _dimensionsLabel->clear( );
    _errorLabel->setText( "<span style=\"color: red;\">Model file went missing</span>" );
    update( );

    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::loader_finished( ) {
    debug( "+ FileTab::loader_finished: %s\n", ToString( _canvas->size( ) ).toUtf8( ).data( ) );

    _availableFilesListView->setEnabled( true );
    _viewSolid->setEnabled( true );
    _viewWireframe->setEnabled( true );
    update( );

    _loader->deleteLater( );
    _loader = nullptr;
}

void FileTab::libraryFsModel_directoryLoaded( QString const& /*name*/ ) {
    debug( "+ FileTab::libraryFsModel_directoryLoaded\n" );
    if ( _modelsLocation == ModelsLocation::Library ) {
        _libraryFsModel->sort( 0, Qt::AscendingOrder );
        _availableFilesListView->setRootIndex( _libraryFsModel->index( StlModelLibraryPath ) );
        update( );
    }
}

void FileTab::usbFsModel_directoryLoaded( QString const& /*name*/ ) {
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

    _modelSelection = { ( ( _modelsLocation == ModelsLocation::Library ) ? StlModelLibraryPath : _usbPath ) % Slash % index.data( ).toString( ) };
    _modelSelection.type = QFileInfo { _modelSelection.fileName }.isFile( ) ? ModelFileType::File : ModelFileType::Directory;

    _selectedRow = indexRow;

    if ( _modelSelection.type == ModelFileType::File ) {
        _availableFilesListView->setEnabled( false );
        _viewSolid->setEnabled( false );
        _viewWireframe->setEnabled( false );
        _selectButton->setEnabled( false );
        _deleteButton->setEnabled( false );
        update( );

        if ( _processRunner ) {
            QObject::disconnect( _processRunner, nullptr, this, nullptr );
            _processRunner->terminate( );
            _processRunner->deleteLater( );
            _processRunner = nullptr;
        }

        _loadModel( _modelSelection.fileName );
    } else {
        _dimensionsLabel->clear( );
        _errorLabel->clear( );
        _viewSolid->setEnabled( false );
        _viewWireframe->setEnabled( false );

        if (!_printManager->isRunning()) {
            _selectButton->setEnabled(true);
            _deleteButton->setEnabled(true);
        }

        QTimer::singleShot( 1, [this] ( ) { _canvas->clear( ); } );
    }
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

void FileTab::selectButton_clicked(bool)
{
    debug(
        "+ FileTab::selectButton_clicked:\n"
        "  + current models location: %s\n"
        "",
        ToString( _modelsLocation )
    );

    if (_modelsLocation == ModelsLocation::Library) {
        if (_modelSelection.type == ModelFileType::File) {
            _printJob->resetTiling();
            emit modelSelected(&_modelSelection);
            emit uiStateChanged(TabIndex::File, UiState::SelectCompleted);
        } else {
            auto match = SliceDirectoryNameRegex.match(_modelSelection.fileName);
            if (match.hasMatch()) {
                _printJob->baseSlices.layerCount = 2;
                _printJob->baseSlices.layerThickness = match.captured(1).toInt();
                _printJob->bodySlices.layerThickness = match.captured(1).toInt();
                _printJob->directoryMode = true;
                _printJob->directoryPath = _modelSelection.fileName;
                emit uiStateChanged(TabIndex::File, UiState::SelectCompleted);
            } else {
                auto match = TiledDirectoryNameRegex.match(_modelSelection.fileName);
                if (match.hasMatch()) {
                    _printJob->directoryMode = true;
                    _printJob->directoryPath = _modelSelection.fileName;
                    emit uiStateChanged(TabIndex::File, UiState::SelectCompleted);
                }
            }

            _printJob->modelFileName = _modelSelection.fileName;
        }
    } else {
        debug( "  + current model file type: %s\n", ToString( _modelSelection.type ) );
        if ( _modelSelection.type == ModelFileType::File ) {
            auto fileCopier   { new FileCopier };
            auto fileNamePair { FileNamePair {
                _modelSelection.fileName,
                StlModelLibraryPath % Slash % GetFileBaseName( _modelSelection.fileName ) }
            };

            QObject::connect( fileCopier, &FileCopier::notify, this, [ this ] ( int const index, QString const message ) {
                debug( "+ FileTab::selectButton_clicked/lambda for FileCopier::notify: index %d: message '%s'\n", index, message.toUtf8( ).data( ) );
            }, Qt::QueuedConnection );

            QObject::connect( fileCopier, &FileCopier::finished, this, [ this, fileNamePair ] ( int const copiedFiles, int const skippedFiles ) {
                debug( "+ FileTab::selectButton_clicked/lambda for FileCopier::finished: files copied %d, files skipped %d\n", copiedFiles, skippedFiles );
                if ( copiedFiles == 1 ) {
                    _showLibrary( );

                    auto index = _libraryFsModel->index( fileNamePair.second );
                    _availableFilesListView->selectionModel( )->select( index, QItemSelectionModel::ClearAndSelect );
                    availableFilesListView_clicked( index );
                } else {
                    // TODO inform user of failure somehow
                }
            }, Qt::QueuedConnection );

            QObject::connect( fileCopier, &FileCopier::finished, fileCopier, &FileCopier::deleteLater, Qt::QueuedConnection );

            debug( "+ FileTab::selectButton_clicked: copying %s to %s\n", fileNamePair.first.toUtf8( ).data( ), fileNamePair.second.toUtf8( ).data( ) );
            fileCopier->copy( { fileNamePair } );
        } else if ( _modelSelection.type == ModelFileType::Directory ) {
            QString folderCpyName { GetFileBaseName( _modelSelection.fileName ) };
            QString folderCpyPath { JobWorkingDirectoryPath % Slash % folderCpyName };

            QDir modelLib { JobWorkingDirectoryPath };
            modelLib.mkdir( folderCpyName );

            QDirIterator iter { _modelSelection.fileName, QDir::Files };
            FileNamePairList pairList;
            while ( iter.hasNext( ) ) {
                QString fileName = iter.next( );
                auto fileNamePair { FileNamePair {
                    fileName,
                    folderCpyPath % Slash % GetFileBaseName( fileName ) }
                };

                pairList.push_back( fileNamePair );
            }
            _selectButton->setEnabled(false);
            FileCopier* fileCopier { new FileCopier };
            QObject::connect( fileCopier, &FileCopier::finished, this, [this, folderCpyPath, folderCpyName] ( int const copiedFiles, int const skippedFiles ) {
                debug( QString("+ FileTab::selectButton_clicked/lambda for FileCopier::finished: files "
                               "copied %1, files skipped %2\n, folder name %3")
                       .arg(copiedFiles).arg(skippedFiles).arg(folderCpyName).toUtf8().data() );
                _selectButton->setEnabled(true);
                _showLibrary( );

                auto index = _libraryFsModel->index( _modelSelection.fileName );
                _availableFilesListView->selectionModel( )->select( index, QItemSelectionModel::ClearAndSelect );
                availableFilesListView_clicked( index );

                QFile::link( folderCpyPath, StlModelLibraryPath % Slash % folderCpyName );
            }, Qt::QueuedConnection );

            QObject::connect( fileCopier, &FileCopier::finished, fileCopier, &FileCopier::deleteLater, Qt::QueuedConnection );
            fileCopier->copy( pairList );
        }
    }

    update( );
}

void FileTab::viewSolid_toggled( bool checked ) {
    debug( "+ FileTab::viewSolid_toggled: %s\n", ToString( checked ) );
    if ( checked ) {
        _canvas->draw_shaded( );
    }
}

void FileTab::viewWireframe_toggled( bool checked ) {
    debug( "+ FileTab::viewWireframe_toggled: %s\n", ToString( checked ) );
    if ( checked ) {
        _canvas->draw_wireframe( );
    }
}

void FileTab::deleteButton_clicked( bool ) {
    debug( "+ FileTab::deleteButton_clicked: file name is '%s'\n", _modelSelection.fileName.toUtf8( ).data( ) );

    App::mainWindow( )->hide( );
    if ( YesNoPrompt( this, "Confirm", QString::asprintf( "Are you sure you want to delete<br /><span style=\"font-weight: bold;\">%s</span><br />from the <span style=\"font-weight: bold;\">%s</span>?", GetFileBaseName( _modelSelection.fileName ).toUtf8( ).data( ), ( _modelsLocation == ModelsLocation::Library ) ? "model library" : "USB stick" ) ) ) {
        ModelFileNameToDelete = _modelSelection.fileName;
        if ( ( _modelsLocation == ModelsLocation::Usb ) && !_usbMountManager->isWritable( ) ) {
            debug( "+ FileTab::deleteButton_clicked: attempting to remount USB stick RW in order to delete model\n" );
            QObject::connect( _usbMountManager, &UsbMountManager::filesystemRemounted, this, &FileTab::usbMountManager_filesystemRemounted );
            _usbMountManager->remount( true );
        } else {
            debug( "+ FileTab::deleteButton_clicked: deleting model directly\n" );
            _deleteModel( );
            App::mainWindow( )->show( );
        }
    } else {
        App::mainWindow( )->show( );
    }
}

void FileTab::processRunner_succeeded( ) {
    TimingLogger::stopTiming( TimingId::VolumeCalculation );
    debug( "+ FileTab::processRunner_succeeded\n" );
    if (!_printManager->isRunning())
        _deleteButton->setEnabled(true);

    for ( auto line : _slicerBuffer.split( NewLineRegex ) ) {
        if ( auto match = VolumeLineMatcher.match( line ); match.hasMatch( ) ) {
            bool ok = false;
            _modelSelection.estimatedVolume = match.captured( 1 ).toDouble( &ok );
            if ( !ok ) {
                debug( "  + couldn't parse '%s' as a number\n", match.captured( 1 ).toUtf8( ).data( ) );
                break;
            }

            QString unit;
            double estimatedVolume = _modelSelection.estimatedVolume;

            debug( "  + Estimated volume of model: %.3f µL\n", estimatedVolume );
            if ( estimatedVolume < 1000.0 ) {
                unit = "µL";
            } else {
                estimatedVolume /= 1000.0;
                unit = "mL";
            }
            _dimensionsLabel->setText( _dimensionsText % Comma % Space % GroupDigits( QString { "%1" }.arg( estimatedVolume, 0, 'f', 2 ), ' ' ) % Space % unit );
            if (!_printManager->isRunning())
                _selectButton->setEnabled(true);

            update( );
            break;
        }
    }

    _slicerBuffer.clear( );
}

void FileTab::processRunner_failed( int const exitCode, QProcess::ProcessError const error ) {
    TimingLogger::stopTiming( TimingId::VolumeCalculation );
    debug( "+ FileTab::processRunner_failed: exit code: %d, error %s [%d]\n", exitCode, ToString( error ), error );
    if (!_printManager->isRunning())
        _deleteButton->setEnabled(true);

    _slicerBuffer.clear( );
    emit uiStateChanged( TabIndex::File, UiState::SelectStarted );
}

void FileTab::processRunner_readyReadStandardOutput( QString const& data ) {
    _slicerBuffer += data;
}

void FileTab::processRunner_readyReadStandardError( QString const& data ) {
    auto tmp = data.toUtf8( );
    fwrite( tmp.data( ), 1, tmp.count( ), stderr );
}
