#include "pch.h"

//#include <sys/sysinfo.h>

#include "preparetab.h"

#include "hasher.h"
#include "printjob.h"
#include "printmanager.h"
#include "printprofile.h"
#include "shepherd.h"
#include "slicesorderpopup.h"
#include "thicknesswindow.h"
#include "svgrenderer.h"
#include "timinglogger.h"
#include "usbmountmanager.h"
#include "window.h"

PrepareTab::PrepareTab( QWidget* parent ): InitialShowEventMixin<PrepareTab, TabBase>( parent ) {
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto font12pt    = ModifyFont( origFont, 14.0 );
    auto font22pt    = ModifyFont( origFont, LargeFontSize );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome" );

    _layerThicknessLabel->setEnabled( false );
    _layerThicknessLabel->setText( "Layer height:" );

    _layerThickness100Button->setEnabled( false );
    _layerThickness100Button->setChecked( true );
    _layerThickness100Button->setFont( font12pt );
    _layerThickness100Button->setText( "Standard res (100 µm)" );
    QObject::connect( _layerThickness100Button, &QPushButton::clicked, this, &PrepareTab::layerThickness100Button_clicked );

    _layerThickness50Button->setEnabled( false );
    _layerThickness50Button->setText( "High res (50 µm)" );
    _layerThickness50Button->setFont( font12pt );
    QObject::connect( _layerThickness50Button, &QPushButton::clicked, this, &PrepareTab::layerThickness50Button_clicked );

    _layerThicknessCustomButton->setEnabled( false );
    _layerThicknessCustomButton->setText( "Custom (advanced tiling)" );
    _layerThicknessCustomButton->setFont( font12pt );
    QObject::connect( _layerThicknessCustomButton, &QPushButton::clicked, this, &PrepareTab::layerThicknessCustomButton_clicked );

#if defined EXPERIMENTAL
    _layerThickness20Button->setEnabled( false );
    _layerThickness20Button->setText( "Super-high res (20 µm)" );
    _layerThickness20Button->setFont( font12pt );
    QObject::connect( _layerThickness20Button, &QPushButton::clicked, this, &PrepareTab::layerThickness20Button_clicked );
#endif

    _sliceStatusLabel->setText( "Slicer:" );

    _sliceStatus->setText( "idle" );
    _sliceStatus->setFont( boldFont );

    _imageGeneratorStatusLabel->setText( "Image generator:" );

    _imageGeneratorStatus->setText( "idle" );
    _imageGeneratorStatus->setFont( boldFont );

    _prepareMessage->setAlignment( Qt::AlignCenter );
    _prepareMessage->setTextFormat( Qt::RichText );
    _prepareMessage->setWordWrap( true );
    _prepareMessage->setText( "Tap the <b>Prepare</b> button to prepare the printer." );

    _prepareProgress->setAlignment( Qt::AlignCenter );
    _prepareProgress->setFormat( { } );
    _prepareProgress->setRange( 0, 0 );
    _prepareProgress->setTextVisible( false );
    _prepareProgress->hide( );

    _prepareGroup->setTitle( "Printer preparation" );
    _prepareGroup->setLayout( WrapWidgetsInVBox(
        nullptr,
        WrapWidgetsInHBox( _prepareMessage ),
        nullptr,
        WrapWidgetsInHBox( _prepareProgress ),
        nullptr
    ) );

    _prepareGroup->setFixedHeight(110);
    _warningHotImage = new QPixmap { QString { ":images/warning-hot.png" } };
    _warningHotLabel->setAlignment( Qt::AlignCenter );
    _warningHotLabel->setContentsMargins( { } );
    _warningHotLabel->setFixedSize( 43, 43 );
    _warningHotLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _warningUvImage = new QPixmap { QString { ":images/warning-uv.png"  } };
    _warningUvLabel->setAlignment( Qt::AlignCenter );
    _warningUvLabel->setContentsMargins( { } );
    _warningUvLabel->setFixedSize( 43, 43 );
    _warningUvLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _prepareButton->setEnabled( false );
    _prepareButton->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _prepareButton->setFont( font22pt );
    _prepareButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _prepareButton->setText( "Prepare" );
    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );

    //_copyToUSBButton->setEnabled( false );
    //_copyToUSBButton->setFixedSize( MainButtonSize );
    //_copyToUSBButton->setFont( font22pt );
    //_copyToUSBButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    //_copyToUSBButton->setText( "Copy to USB" );
    //QObject::connect( _copyToUSBButton, &QPushButton::clicked, this, &PrepareTab::copyToUSB_clicked );

    _optionsContainer->setFixedWidth( MainButtonSize.width( ) );
    _optionsContainer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _optionsContainer->setLayout( WrapWidgetsInVBox(
        _layerThicknessLabel,
#if defined EXPERIMENTAL
         WrapWidgetsInVBox(
             _layerThickness100Button,
             _layerThickness50Button,
             _layerThickness20Button,
             _layerThicknessCustomButton
         ),
#else
          WrapWidgetsInVBox(
              _layerThickness100Button,
              _layerThickness50Button,
              _layerThicknessCustomButton
          ),
#endif // defined EXPERIMENTAL
        WrapWidgetsInHBox( _sliceStatusLabel,          nullptr, _sliceStatus          ),
        WrapWidgetsInHBox( _imageGeneratorStatusLabel, nullptr, _imageGeneratorStatus ),
        WrapWidgetsInVBox(
            WrapWidgetsInHBox( nullptr, _warningHotLabel, nullptr, _warningUvLabel, nullptr )
        )
    ) );

    _sliceButton->setEnabled( false );
    _sliceButton->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _sliceButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _sliceButton->setFont( font22pt );
    _sliceButton->setText( "Slice" );
    QObject::connect( _sliceButton, &QPushButton::clicked, this, &PrepareTab::sliceButton_clicked );

    _orderButton->setEnabled( false );
    _orderButton->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _orderButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _orderButton->setFont( font22pt );
    _orderButton->setText( "Order editor" );
    QObject::connect( _orderButton, &QPushButton::clicked, this, &PrepareTab::orderButton_clicked );

    _setupTiling->setEnabled( false );
    _setupTiling->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _setupTiling->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _setupTiling->setFont( font22pt );
    _setupTiling->setText( "Setup tiling" );
    QObject::connect( _setupTiling, &QPushButton::clicked, this, &PrepareTab::setupTiling_clicked );

    _currentLayerImage->setAlignment( Qt::AlignCenter );
    _currentLayerImage->setContentsMargins( { } );
    _currentLayerImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentLayerImage->setStyleSheet( "QWidget { background: black }" );

    for ( auto button : { _navigateFirst, _navigatePrevious, _navigateNext, _navigateLast } ) {
        button->setFont( fontAwesome );
    }

    _navigateFirst   ->setText( FA_FastBackward );
    _navigatePrevious->setText( FA_Backward     );
    _navigateNext    ->setText( FA_Forward      );
    _navigateLast    ->setText( FA_FastForward  );

    QObject::connect( _navigateFirst,    &QPushButton::clicked, this, &PrepareTab::navigateFirst_clicked    );
    QObject::connect( _navigatePrevious, &QPushButton::clicked, this, &PrepareTab::navigatePrevious_clicked );
    QObject::connect( _navigateNext,     &QPushButton::clicked, this, &PrepareTab::navigateNext_clicked     );
    QObject::connect( _navigateLast,     &QPushButton::clicked, this, &PrepareTab::navigateLast_clicked     );

    _navigateCurrentLabel->setAlignment( Qt::AlignCenter );
    _navigateCurrentLabel->setText( "0/0" );

    _navigationLayout = WrapWidgetsInHBox( nullptr, _navigateFirst, _navigatePrevious, _navigateCurrentLabel, _navigateNext, _navigateLast, nullptr );
    _navigationLayout->setAlignment( Qt::AlignCenter );

    _setNavigationButtonsEnabled( false );

    _currentLayerLayout = WrapWidgetsInVBox(
        _currentLayerImage,
        _navigationLayout
    );
    _currentLayerLayout->setAlignment( Qt::AlignTop | Qt::AlignHCenter );

    _currentLayerGroup->setTitle( "Current layer" );
    _currentLayerGroup->setMinimumSize( MaximalRightHandPaneSize );
    _currentLayerGroup->setLayout( _currentLayerLayout );

    _layout->setContentsMargins( { } );
    _layout->addWidget( _optionsContainer, 0, 0, 1, 1 );
    _layout->addWidget( _prepareButton, 1, 0, 1, 1 );
    _layout->addWidget( _orderButton, 2, 0, 1, 1 );
    _layout->addWidget( _setupTiling, 3, 0, 1, 1 );
    _layout->addWidget( _sliceButton, 4, 0, 1, 1 );
    _layout->addWidget( _currentLayerGroup, 0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

PrepareTab::~PrepareTab( ) {
    /*empty*/
}

void PrepareTab::_connectPrintManager( ) {
    if ( _printManager ) {
        QObject::connect( _printManager, &PrintManager::lampStatusChange, this, &PrepareTab::printManager_lampStatusChange );
    }
}

void PrepareTab::_connectShepherd( ) {
    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_online,            this, &PrepareTab::printer_online            );
        QObject::connect( _shepherd, &Shepherd::printer_offline,           this, &PrepareTab::printer_offline           );
        QObject::connect( _shepherd, &Shepherd::printer_temperatureReport, this, &PrepareTab::printer_temperatureReport );
    }
}

void PrepareTab::_initialShowEvent( QShowEvent* event ) {
    _currentLayerImage->setFixedSize( _currentLayerImage->width( ), _currentLayerImage->width( ) / AspectRatio16to10 + 0.5 );
    update( );

    event->accept( );
}

void PrepareTab::_connectUsbMountManager( ) {
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemMounted,   this, &PrepareTab::usbMountManager_filesystemMounted   );
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemUnmounted, this, &PrepareTab::usbMountManager_filesystemUnmounted );
}

bool PrepareTab::_checkPreSlicedFiles( ) {
    debug( "+ PrepareTab::_checkPreSlicedFiles\n" );

    // check that the sliced SVG file is newer than the STL file
    auto modelFile = QFileInfo { _printJob->modelFileName };
    if ( !modelFile.exists( ) ) {
        debug( "  + Fail: model file does not exist\n" );
        return false;
    }

    auto slicedSvgFile = QFileInfo { _printJob->sliceDirectory + Slash + SlicedBodySvgFileName };
    if ( !slicedSvgFile.exists( ) ) {
        debug( "  + Fail: sliced body SVG file does not exist\n" );
        return false;
    }

    // check that the sliced body SVG file is newer than the STL file
    auto slicedSvgFileLastModified = slicedSvgFile.lastModified( );
    if ( !_printJob->modelFileName.isEmpty( ) ) {
        auto modelFile = QFileInfo { _printJob->modelFileName };
        if ( !modelFile.exists( ) ) {
            debug( "  + Fail: model file does not exist\n" );
            return false;
        }
        if ( !_printJob->modelFileName.isEmpty( ) && ( modelFile.lastModified( ) > slicedSvgFileLastModified ) ) {
            debug( "  + Fail: model file is newer than sliced body SVG file\n" );
            return false;
        }
    }

    _manifestManager->restart();
    _manifestManager->setPath( _printJob->sliceDirectory );
    QStringList errors;
    QStringList warnings;

    switch(_manifestManager->parse(&errors, &warnings))
    {
    case ManifestParseResult::POSITIVE_WITH_WARNINGS: {
        QString warningsStr = warnings.join("<br>");

            _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually: " % warningsStr);
        }
    case ManifestParseResult::POSITIVE:
        if(_manifestManager->tiled()){

            debug( "+ PrepareTab::_checkPreSlicedFiles ManifestParseResult::POSITIVE\n" );

            _setupTiling->setEnabled( false );
            _printJob->estimatedVolume = _manifestManager->tiledVolume();
        }
        break;
    case ManifestParseResult::FILE_CORRUPTED:
    case ManifestParseResult::FILE_NOT_EXIST: {
            QString errorsStr = errors.join("<br>");
            _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually. <br>" % errorsStr);

            SlicesOrderPopup slicesOrderPopup { _manifestManager };
            slicesOrderPopup.exec();

            break;
        }
    }

    _printJob->totalLayerCount = _manifestManager->getSize();

    OrderManifestManager::Iterator iter = _manifestManager->iterator();

    int baseLayerCount = 0;
    int bodyLayerCount = 0;
    int currentLayer = 0;

    // check that the layer SVG files are newer than the sliced SVG file,
    //   and that the layer PNG files are newer than the layer SVG files,
    //   and that there are no gaps in the numbering.
    while ( iter.hasNext() ) {

        QFileInfo entry ( _printJob->sliceDirectory % Slash % *iter);
        ++iter;

        if ( slicedSvgFileLastModified > entry.lastModified( ) ) {
            debug( "  + Fail: sliced SVG file is newer than layer SVG file %s\n", entry.fileName( ).toUtf8( ).data( ) );
            return false;
        }

        auto layerPngFile = QFileInfo { entry.path( ) + Slash + entry.completeBaseName( ) + QString { ".png" } };
        if ( !layerPngFile.exists( ) ) {
            debug( "  + Fail: layer PNG file %s does not exist\n", layerPngFile.fileName( ).toUtf8( ).data( ) );
            return false;
        }
        if ( entry.lastModified( ) > layerPngFile.lastModified( ) ) {
            debug( "  + Fail: layer SVG file %s is newer than layer PNG file %s\n", entry.fileName( ).toUtf8( ).data( ), layerPngFile.fileName( ).toUtf8( ).data( ) );
            return false;
        }

        if ( _manifestManager->isBaseLayer( currentLayer ) ) {
            baseLayerCount++;
        } else {
            bodyLayerCount++;
        }
        currentLayer++;
    }

    if (baseLayerCount > 0) {
        auto slicedSvgFile = QFileInfo { _printJob->sliceDirectory + Slash + SlicedBaseSvgFileName };
        if ( !slicedSvgFile.exists( ) ) {
            debug( "  + Fail: sliced base SVG file does not exist\n" );
            return false;
        }

        // check that the sliced base SVG file is newer than the STL file
        auto slicedSvgFileLastModified = slicedSvgFile.lastModified( );
        if ( !_printJob->modelFileName.isEmpty( ) ) {
            auto modelFile = QFileInfo { _printJob->modelFileName };
            if ( !modelFile.exists( ) ) {
                debug( "  + Fail: model file does not exist\n" );
                return false;
            }
            if ( !_printJob->modelFileName.isEmpty( ) && ( modelFile.lastModified( ) > slicedSvgFileLastModified ) ) {
                debug( "  + Fail: model file is newer than sliced base SVG file\n" );
                return false;
            }
        }
    }

    _printJob->baseSlices.layerCount = baseLayerCount;
    _printJob->bodySlices.layerCount = bodyLayerCount;
    debug( "  + Success: %d base layers, %d body layers\n", _printJob->baseSlices.layerCount, _printJob->bodySlices.layerCount );

    return true;
}

bool PrepareTab::_checkSliceDirectory(  ) {
    QString sliceDirectoryBase { JobWorkingDirectoryPath % Slash % _printJob->modelHash };
    _printJob->sliceDirectory = sliceDirectoryBase;

    debug(
        "+ PrepareTab::_checkSliceDirectory:"
        "  + model filename:        '%s'\n"
        "  + slices directory:      '%s'\n"
        "",
        _printJob->modelFileName.toUtf8( ).data( ),
        _printJob->sliceDirectory.toUtf8( ).data( )
    );

    if ( QDir slicesDir { _printJob->sliceDirectory }; !slicesDir.exists( ) ) {
        _printJob->isPreSliced = false;
        debug( "  + no pre-sliced layers\n");
    } else {
        _printJob->isPreSliced = _checkPreSlicedFiles( );
        debug( "  + pre-sliced layers are %sgood\n", _printJob->isPreSliced ? "" : "NOT " );
        if ( !_printJob->isPreSliced ) {
            slicesDir.removeRecursively( );
        }
    }

    _setNavigationButtonsEnabled( _printJob->isPreSliced );
    _setSliceControlsEnabled( true );
    if ( _printJob->isPreSliced ) {
        if ( _printJob->printProfile->baseLayerCount( ) > 0 ) {
            int baseThickness                      = _printJob->baseSlices.layerThickness * _printJob->printProfile->baseLayerCount( );

            _printJob->baseSlices.startLayer       = 0;
            _printJob->baseSlices.endLayer         = _printJob->printProfile->baseLayerCount( ) - 1;

            _printJob->bodySlices.firstLayerOffset = ( baseThickness % _printJob->bodySlices.layerThickness ) - _printJob->bodySlices.layerThickness;
            _printJob->bodySlices.startLayer       = ( baseThickness / _printJob->bodySlices.layerThickness ) - 1;
            _printJob->bodySlices.endLayer         = _printJob->bodySlices.layerCount - 1;

            _printJob->totalLayerCount             = _printJob->printProfile->baseLayerCount( ) + ( _printJob->bodySlices.endLayer - _printJob->bodySlices.startLayer + 1 );
        } else {
            _printJob->baseSlices.startLayer       = -1;
            _printJob->baseSlices.endLayer         = -1;

            _printJob->bodySlices.startLayer       =  0;
            _printJob->bodySlices.endLayer         = _printJob->bodySlices.layerCount - 1;

            _printJob->totalLayerCount             = _printJob->bodySlices.layerCount;
        }

        _navigateCurrentLabel->setText( QString { "1/%1" }.arg( _printJob->totalLayerCount ) );
        _sliceButton->setText( "Reslice" );
        _setupTiling->setEnabled(true);
        _orderButton->setEnabled(false);
        //_copyToUSBButton->setEnabled( true );
    } else {
        _printJob->totalLayerCount = 0;
        _navigateCurrentLabel->setText( "0/0" );
        _sliceButton->setText( "Slice" );
        _setupTiling->setEnabled(false);
        _orderButton->setEnabled(false);
        //_copyToUSBButton->setEnabled( false );
    }

    update( );
    return _printJob->isPreSliced;
}

void PrepareTab::layerThickness100Button_clicked( bool ) {
    debug( "+ PrepareTab::layerThickness100Button_clicked\n" );
    _printJob->baseSlices.layerThickness = 0;
    _printJob->bodySlices.layerThickness = 100;

    _checkSliceDirectory( );
}

void PrepareTab::layerThickness50Button_clicked( bool ) {
    debug( "+ PrepareTab::layerThickness50Button_clicked\n" );
    _printJob->baseSlices.layerThickness = 0;
    _printJob->bodySlices.layerThickness = 50;

    _checkSliceDirectory( );
}

#if defined EXPERIMENTAL
void PrepareTab::layerThickness20Button_clicked( bool ) {
    debug( "+ PrepareTab::layerThickness20Button_clicked\n" );
    _printJob->baseSlices.layerThickness = 0;
    _printJob->bodySlices.layerThickness = 20;

    _checkSliceDirectory( );
}
#endif // defined EXPERIMENTAL

void PrepareTab::layerThicknessCustomButton_clicked( bool ) {
    ThicknessWindow *dialog = new ThicknessWindow(_printJob, this);
    switch (dialog->exec()) {
    case QDialog::Rejected:
        _layerThicknessCustomButton->setChecked(false);
        _layerThickness100Button->setChecked(true);
    }
}

void PrepareTab::_setNavigationButtonsEnabled( bool const enabled ) {
    _navigateFirst   ->setEnabled( enabled && ( _visibleLayer > 0 ) );
    _navigatePrevious->setEnabled( enabled && ( _visibleLayer > 0 ) );
    _navigateNext    ->setEnabled( enabled && ( _printJob && ( _visibleLayer + 1 < _printJob->totalLayerCount ) ) );
    _navigateLast    ->setEnabled( enabled && ( _printJob && ( _visibleLayer + 1 < _printJob->totalLayerCount ) ) );

    update( );
}

void PrepareTab::_showLayerImage( int const layer ) {
    QPixmap pixmap { _printJob->getLayerFileName( layer ) };
    if ( ( pixmap.width( ) > _currentLayerImage->width( ) ) || ( pixmap.height( ) > _currentLayerImage->height( ) ) ) {
        pixmap = pixmap.scaled( _currentLayerImage->size( ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }

    _currentLayerImage->setPixmap( pixmap );
    _navigateCurrentLabel->setText( QString { "%1/%2" }.arg( layer + 1 ).arg( _printJob->totalLayerCount ) );

    update( );
}

void PrepareTab::_setSliceControlsEnabled( bool const enabled ) {
    _sliceButton->setEnabled( enabled );

    if(!_directoryMode) {
        _orderButton->setEnabled( false );
    } else {
        _orderButton->setEnabled( enabled );
    }

    _setupTiling->setEnabled( enabled && !_manifestManager->tiled() );

    _layerThicknessLabel->setEnabled( enabled );
    _layerThickness100Button->setEnabled( enabled );
    _layerThickness50Button->setEnabled( enabled );
    _layerThicknessCustomButton->setEnabled( enabled );
#if defined EXPERIMENTAL
    _layerThickness20Button->setEnabled( enabled );
#endif // defined EXPERIMENTAL

    update( );
}

void PrepareTab::_updatePrepareButtonState( ) {
    _prepareButton->setEnabled( _isPrinterOnline && _isPrinterAvailable );

    update( );
}

void PrepareTab::_handlePrepareFailed( ) {
    _prepareMessage->setText( "Preparation failed." );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );

    _prepareButton->setText( "Retry" );
    _prepareButton->setEnabled( true );

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );
    emit preparePrinterComplete( false );

    update( );
}

void PrepareTab::_startSlicer( SliceInformation const& sliceInfo ) {
    TimingLogger::startTiming( TimingId::SlicingSvg, GetFileBaseName( _printJob->modelFileName ) );

    QString outputFile;
    if ( _printJob->isBaseLayer( sliceInfo.startLayer ) ) {
        outputFile = _printJob->sliceDirectory % Slash % SlicedBaseSvgFileName;
    } else {
        outputFile = _printJob->sliceDirectory % Slash % SlicedBodySvgFileName;
    }

    debug( "  + pre-sliced layers are %sgood\n", _printJob->isPreSliced ? "" : "NOT " );

    QStringList slicerArgs = {
        _printJob->modelFileName,
        "--export-svg",
        "--threads",            QString { "%1" }.arg( get_nprocs( ) ),
        "--first-layer-height", QString { "%1" }.arg( ( sliceInfo.layerThickness + sliceInfo.firstLayerOffset ) / 1000.0 ),
        "--layer-height",       QString { "%1" }.arg(   sliceInfo.layerThickness                                / 1000.0 ),
        "--output",             outputFile
    };

    debug("slic3r arguments are: %s\n", slicerArgs.join(" ").toUtf8().constData());

    _slicerProcess->start(
        { "slic3r" },
        slicerArgs
    );
}

void PrepareTab::navigateFirst_clicked( bool ) {
    _visibleLayer = 0;
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::navigatePrevious_clicked( bool ) {
    if ( _visibleLayer > 0 ) {
        --_visibleLayer;
    }
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::navigateNext_clicked( bool ) {
    if ( _visibleLayer + 1 < _printJob->totalLayerCount ) {
        ++_visibleLayer;
    }
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::navigateLast_clicked( bool ) {
    _visibleLayer = _printJob->totalLayerCount - 1;
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::orderButton_clicked( bool ) {
    SlicesOrderPopup popup { _manifestManager };
    popup.exec();

    if(_directoryMode)
        emit  uiStateChanged(TabIndex::File, UiState::SelectedDirectory);
    else
        emit uiStateChanged(TabIndex::File, UiState::SelectCompleted);
}

void PrepareTab::setupTiling_clicked( bool ) {
    emit setupTiling( _manifestManager, _printJob );
    emit uiStateChanged( TabIndex::Prepare, UiState::TilingClicked );
}

void PrepareTab::sliceButton_clicked( bool ) {
    debug( "+ PrepareTab::sliceButton_clicked\n" );
    QDir jobDir { _printJob->sliceDirectory };
    jobDir.removeRecursively( );
    jobDir.mkdir( _printJob->sliceDirectory );

    _sliceStatus->setText( "starting base layers" );
    _imageGeneratorStatus->setText( "waiting" );

    TimingLogger::startTiming( TimingId::SlicingSvg, GetFileBaseName( _printJob->modelFileName ) );

    _manifestManager->removeManifest();

    if ( _printJob->baseSlices.startLayer == -1 ) {
        slicerProcess_base_finished(0, QProcess::ExitStatus::NormalExit);
        return;
    }

    _slicerProcess = new QProcess( this );
    _slicerProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    QObject::connect( _slicerProcess, &QProcess::errorOccurred,                                        this, &PrepareTab::slicerProcess_base_errorOccurred );
    QObject::connect( _slicerProcess, &QProcess::started,                                              this, &PrepareTab::slicerProcess_base_started       );
    QObject::connect( _slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrepareTab::slicerProcess_base_finished      );

    _startSlicer( _printJob->baseSlices );
    _setSliceControlsEnabled( false );
    emit uiStateChanged( TabIndex::Prepare, UiState::SliceStarted );

    update( );
}

void PrepareTab::hasher_resultReady( QString const hash ) {
    debug(
        "+ PrepareTab::hasher_resultReady:\n"
        "  + result hash:           '%s'\n"
        "",
        hash.toUtf8( ).data( )
    );

    _printJob->modelHash = hash.isEmpty( ) ? QString( "%1-%2" ).arg( time( nullptr ) ).arg( getpid( ) ) : hash;

    _sliceStatus->setText( "idle" );
    _hasher = nullptr;

    bool goodJobDir = _checkSliceDirectory( );
    emit slicingNeeded( !goodJobDir );

    update( );
}

void PrepareTab::slicerProcess_base_errorOccurred( QProcess::ProcessError error ) {
    debug( "+ PrepareTab::slicerProcess_base_errorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        debug( "  + slicer process failed to start\n" );
        _sliceStatus->setText( "failed to start" );
    } else if ( QProcess::Crashed == error ) {
        debug( "  + slicer process crashed? state is %s [%d]\n", ToString( _slicerProcess->state( ) ), _slicerProcess->state( ) );
        if ( _slicerProcess->state( ) != QProcess::NotRunning ) {
            _slicerProcess->kill( );
            debug( "  + slicer terminated\n" );
        }
        _sliceStatus->setText( "crashed" );
    }

    update( );
}

void PrepareTab::slicerProcess_base_started( ) {
    debug( "+ PrepareTab::slicerProcess_base_started\n" );
    _sliceStatus->setText( "slicing base layers" );

    update( );
}

void PrepareTab::slicerProcess_base_finished( int exitCode, QProcess::ExitStatus exitStatus ) {
    TimingLogger::stopTiming( TimingId::SlicingSvg );
    debug( "+ PrepareTab::slicerProcess_base_finished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    _slicerProcess->deleteLater( );
    _slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        debug( "  + slicer process crashed?\n" );
        _sliceStatus->setText( "crashed" );
        emit uiStateChanged( TabIndex::Prepare, UiState::SelectCompleted );

        update( );
        return;
    } else if ( ( exitStatus == QProcess::NormalExit ) && ( exitCode != 0 ) ) {
        debug( "  + slicer process failed\n" );
        _sliceStatus->setText( "failed" );
        emit uiStateChanged( TabIndex::Prepare, UiState::SelectCompleted );

        update( );
        return;
    }

    _sliceStatus->setText( "starting body layers" );
    _imageGeneratorStatus->setText( "waiting" );

    _slicerProcess = new QProcess( this );
    _slicerProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    QObject::connect( _slicerProcess, &QProcess::errorOccurred,                                        this, &PrepareTab::slicerProcess_body_errorOccurred );
    QObject::connect( _slicerProcess, &QProcess::started,                                              this, &PrepareTab::slicerProcess_body_started       );
    QObject::connect( _slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrepareTab::slicerProcess_body_finished      );

    _startSlicer( _printJob->bodySlices );

    update( );
}

void PrepareTab::slicerProcess_body_errorOccurred( QProcess::ProcessError error ) {
    debug( "+ PrepareTab::slicerProcess_body_errorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        debug( "  + slicer process failed to start\n" );
        _sliceStatus->setText( "failed to start" );
    } else if ( QProcess::Crashed == error ) {
        debug( "  + slicer process crashed? state is %s [%d]\n", ToString( _slicerProcess->state( ) ), _slicerProcess->state( ) );
        if ( _slicerProcess->state( ) != QProcess::NotRunning ) {
            _slicerProcess->kill( );
            debug( "  + slicer terminated\n" );
        }
        _sliceStatus->setText( "crashed" );
    }

    update( );
}

void PrepareTab::slicerProcess_body_started( ) {
    debug( "+ PrepareTab::slicerProcess_body_started\n" );
    _sliceStatus->setText( "slicing body layers" );

    update( );
}

void PrepareTab::_showWarning(QString content) {
    auto origFont    = font( );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome" );

    Window* w = App::mainWindow();
    QRect r = w->geometry();

    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setFont(fontAwesome);
    msgBox.setText(content);
    msgBox.show();
    msgBox.move(r.x() + ((r.width() - msgBox.width())/2), r.y() + ((r.height() - msgBox.height())/2) );
    msgBox.exec();
}


void PrepareTab::slicerProcess_body_finished( int exitCode, QProcess::ExitStatus exitStatus ) {
    TimingLogger::stopTiming( TimingId::SlicingSvg );
    debug( "+ PrepareTab::slicerProcess_body_finished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    _slicerProcess->deleteLater( );
    _slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        debug( "  + slicer process crashed?\n" );
        _sliceStatus->setText( "crashed" );
        emit uiStateChanged( TabIndex::Prepare, UiState::SelectCompleted );

        update( );
        return;
    } else if ( ( exitStatus == QProcess::NormalExit ) && ( exitCode != 0 ) ) {
        debug( "  + slicer process failed\n" );
        _sliceStatus->setText( "failed" );
        emit uiStateChanged( TabIndex::Prepare, UiState::SelectCompleted );

        update( );
        return;
    }

    _sliceStatus->setText( "finished" );
    _imageGeneratorStatus->setText( "starting base layers" );
    //_copyToUSBButton->setEnabled( true );

    update( );

    _svgRenderer = new SvgRenderer;
    QObject::connect( _svgRenderer, &SvgRenderer::layerCount,    this, &PrepareTab::svgRenderer_base_layerCount    );
    QObject::connect( _svgRenderer, &SvgRenderer::layerComplete, this, &PrepareTab::svgRenderer_base_layerComplete );
    QObject::connect( _svgRenderer, &SvgRenderer::done,          this, &PrepareTab::svgRenderer_base_done          );

    _manifestManager->restart();
    _manifestManager->setPath( _printJob->sliceDirectory );
    if ( _directoryMode ) {
        QStringList errors;
        QStringList warnings;

        switch(_manifestManager->parse(&errors, &warnings))
        {
        case ManifestParseResult::POSITIVE_WITH_WARNINGS: {
            QString warningsStr = warnings.join("<br>");

                _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually: " % warningsStr);
            }
        case ManifestParseResult::POSITIVE:
            if(_manifestManager->tiled()) {
                _setupTiling->setEnabled( false );
                // in case of tiled design volume comes from manifest file instead of model calculation
                _printJob->estimatedVolume = _manifestManager->tiledVolume();
            }
            break;
        case ManifestParseResult::FILE_CORRUPTED:
        case ManifestParseResult::FILE_NOT_EXIST: {
                QString errorsStr = errors.join("<br>");
                _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually. <br>" % errorsStr);

                SlicesOrderPopup slicesOrderPopup { _manifestManager };
                slicesOrderPopup.exec();

                break;
            }
        }

        _printJob->totalLayerCount = _manifestManager->getSize();
        _orderButton->setEnabled( true );
        _setupTiling->setEnabled( true );
        _svgRenderer->loadSlices( _manifestManager );
    } else {
        if ( _manifestManager->baseLayersCount() > 0 ) {
            _svgRenderer->startRender( _printJob->sliceDirectory + Slash + SlicedBaseSvgFileName, _printJob->sliceDirectory, _manifestManager);
        } else {
            svgRenderer_base_done(true);
        }
    }
    update();
}


void PrepareTab::svgRenderer_base_layerCount( int const totalLayers ) {
    debug( "+ PrepareTab::svgRenderer_base_layerCount: totalLayers %d\n", totalLayers );
    _printJob->baseSlices.layerCount = totalLayers;

    _navigateCurrentLabel->setText( QString( "1/%1" ).arg( _printJob->baseSlices.layerCount ) );
    update( );
}

void PrepareTab::svgRenderer_base_layerComplete( int const currentLayer ) {
    _imageGeneratorStatus->setText( QString( "layer %1" ).arg( currentLayer + 1 ) );

    if ( ( 0 == currentLayer ) || ( 0 == ( ( currentLayer + 1 ) % 5 ) ) || ( ( _printJob->totalLayerCount - 1 ) == currentLayer ) ) {
        _visibleLayer = currentLayer;
        _showLayerImage( _visibleLayer );
    }
    update( );
}

void PrepareTab::svgRenderer_base_done( bool const success ) {
    _imageGeneratorStatus->setText( success ? "finished" : "failed" );

    QObject::disconnect( _svgRenderer, nullptr, this, nullptr );
    _svgRenderer->deleteLater( );
    _svgRenderer = new SvgRenderer;
    QObject::connect( _svgRenderer, &SvgRenderer::layerCount,    this, &PrepareTab::svgRenderer_body_layerCount    );
    QObject::connect( _svgRenderer, &SvgRenderer::layerComplete, this, &PrepareTab::svgRenderer_body_layerComplete );
    QObject::connect( _svgRenderer, &SvgRenderer::done,          this, &PrepareTab::svgRenderer_body_done          );
    _svgRenderer->startRender( _printJob->sliceDirectory + Slash + SlicedBodySvgFileName, _printJob->sliceDirectory, _manifestManager, _printJob->baseSlices.layerCount );

    update( );
}

void PrepareTab::svgRenderer_body_layerCount( int const totalLayers ) {
    debug( "+ PrepareTab::svgRenderer_body_layerCount: totalLayers %d\n", totalLayers );
    _printJob->bodySlices.layerCount = totalLayers - _printJob->baseSlices.layerCount;
    _printJob->totalLayerCount = totalLayers;

    _navigateCurrentLabel->setText( QString( "1/%1" ).arg( totalLayers ) );
    update( );
}

void PrepareTab::svgRenderer_body_layerComplete( int const currentLayer ) {
    _imageGeneratorStatus->setText( QString( "layer %1" ).arg( currentLayer + 1 ) );

    if ( ( 0 == currentLayer ) || ( 0 == ( ( currentLayer + 1 ) % 5 ) ) || ( ( _printJob->totalLayerCount - 1 ) == currentLayer ) ) {
        _visibleLayer = currentLayer;
        _showLayerImage( _visibleLayer );
    }

    update( );
}

void PrepareTab::svgRenderer_body_done( bool const success ) {
    _imageGeneratorStatus->setText( success ? "finished" : "failed" );

    QObject::disconnect( _svgRenderer, nullptr, this, nullptr );
    _svgRenderer->deleteLater( );
    _svgRenderer = nullptr;

    _setNavigationButtonsEnabled( success );
    _sliceButton->setText( success ? "Reslice" : "Slice" );

    emit uiStateChanged( TabIndex::Prepare, success ? UiState::SliceCompleted : UiState::SelectCompleted );

    update( );
}

void PrepareTab::prepareButton_clicked( bool ) {
    debug( "+ PrepareTab::prepareButton_clicked\n" );

    QObject::disconnect( _prepareButton, &QPushButton::clicked, this, nullptr );

    _prepareMessage->setText( "Moving the build platform to its home location…" );
    _prepareProgress->show( );

    _prepareButton->setText( "Continue" );
    _prepareButton->setEnabled( false );

    QObject::connect( _shepherd, &Shepherd::action_homeComplete, this, &PrepareTab::shepherd_homeComplete );
    _shepherd->doHome( );

    setPrinterAvailable( false );
    emit printerAvailabilityChanged( false );
    emit preparePrinterStarted( );

    update( );
}

void PrepareTab::shepherd_homeComplete( bool const success ) {
    debug( "+ PrepareTab::shepherd_homeComplete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    if ( !success ) {
        _handlePrepareFailed( );
        return;
    }

    _prepareMessage->setText( "Adjust the build platform position, then tap <b>Continue</b>." );

    QObject::disconnect( _prepareButton, &QPushButton::clicked, this, nullptr                                   );
    QObject::connect   ( _prepareButton, &QPushButton::clicked, this, &PrepareTab::adjustBuildPlatform_complete );
    _prepareButton->setEnabled( true );

    update( );
}

void PrepareTab::adjustBuildPlatform_complete( bool ) {
    debug( "+ PrepareTab::adjustBuildPlatform_complete\n" );

    QObject::disconnect( _prepareButton, &QPushButton::clicked, this, nullptr );
    _prepareButton->setEnabled( false );

    _prepareMessage->setText( "Raising the build platform…" );
    _prepareProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrepareTab::shepherd_raiseBuildPlatformMoveToComplete );
    _shepherd->doMoveAbsolute( PrinterRaiseToMaximumZ, PrinterDefaultHighSpeed );

    update( );
}

void PrepareTab::shepherd_raiseBuildPlatformMoveToComplete( bool const success ) {
    debug( "+ PrepareTab::shepherd_raiseBuildPlatformMoveToComplete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );
    _prepareButton->setEnabled( true );

    if ( !success ) {
        _handlePrepareFailed( );
        return;
    }

    _prepareMessage->setText( "Preparation completed." );
    _prepareButton->setText( "Prepare" );

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );
    emit preparePrinterComplete( true );

    update( );
}

void PrepareTab::_updateSliceControls() {
    _layerThickness100Button->setChecked(false);
    _layerThickness50Button->setChecked(false);
    _layerThicknessCustomButton->setChecked(false);
#if defined EXPERIMENTAL
    _layerThickness20Button->setChecked(false);
#endif

    if (_printJob->baseSlices.layerThickness == _printJob->bodySlices.layerThickness &&
        _printJob->baseSlices.layerCount == 2) {
        switch (_printJob->baseSlices.layerThickness) {
        case 100:
            _layerThickness100Button->setChecked(true);
            return;

        case 50:
            _layerThickness50Button->setChecked(true);
            return;

#if defined EXPERIMENTAL
        case 20:
            _layerThicknessCustomButton->setChecked(true);
            return;
#endif
        }
    }

    _layerThicknessCustomButton->setChecked(true);
}

void PrepareTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ PrepareTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch ( _uiState ) {
        case UiState::TilingClicked:
            break;
        case UiState::SelectStarted:
            _directoryMode = false;
            _setSliceControlsEnabled( false );
            break;

        case UiState::SelectCompleted:
            _directoryMode = false;
            _setSliceControlsEnabled( false );

            _sliceStatus->setText( "idle" );
            _imageGeneratorStatus->setText( "idle" );
            _currentLayerImage->clear( );
            _navigateCurrentLabel->setText( "0/0" );
            _setNavigationButtonsEnabled( false );

            if ( _hasher ) {
                _hasher->deleteLater( );
            }
            _hasher = new Hasher;
            QObject::connect( _hasher, &Hasher::resultReady, this, &PrepareTab::hasher_resultReady, Qt::QueuedConnection );
            _hasher->hash( _printJob->modelFileName, QCryptographicHash::Md5 );
            break;

        case UiState::SliceStarted:
            _setSliceControlsEnabled( false );
            break;

        case UiState::SliceCompleted:
            if ( !_directoryMode ) {
                _setSliceControlsEnabled( true );
            }
            break;

        case UiState::PrintStarted:
            _setSliceControlsEnabled( false );
            setPrinterAvailable( false );
            emit printerAvailabilityChanged( false );
            break;

        case UiState::PrintCompleted:
            _setSliceControlsEnabled( true );
            setPrinterAvailable( true );
            emit printerAvailabilityChanged( true );
            break;

        case UiState::SelectedDirectory:
            _directoryMode = true;
            _setSliceControlsEnabled( false );
            _updateSliceControls();
            slicerProcess_body_finished( 0, QProcess::NormalExit );
            break;
    }

    update( );
}

void PrepareTab::printer_online( ) {
    _isPrinterOnline = true;
    debug( "+ PrepareTab::printer_online: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updatePrepareButtonState( );
}

void PrepareTab::printer_offline( ) {
    _isPrinterOnline = false;
    debug( "+ PrepareTab::printer_offline: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updatePrepareButtonState( );
}

void PrepareTab::printer_temperatureReport( double const bedCurrentTemperature, double const, int const ) {
    if ( bedCurrentTemperature >= 30.0 ) {
        _warningHotLabel->setPixmap( *_warningHotImage );
    } else {
        _warningHotLabel->clear( );
    }

    update( );
}

void PrepareTab::printManager_lampStatusChange( bool const on ) {
    if ( on ) {
        _warningUvLabel->setPixmap( *_warningUvImage );
    } else {
        _warningUvLabel->clear( );
    }

    update( );
}

void PrepareTab::setPrinterAvailable( bool const value ) {
    _isPrinterAvailable = value;
    debug( "+ PrepareTab::setPrinterAvailable: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updatePrepareButtonState( );
}

void PrepareTab::loadPrintProfile(PrintProfile const* profile) {
    _printJob->baseSlices.layerCount = profile->baseLayerCount();
    _printJob->baseSlices.layerThickness = profile->baseLayerParameters().layerThickness();
    _printJob->bodySlices.layerThickness = profile->bodyLayerParameters().layerThickness();
    _updateSliceControls();
}

//void PrepareTab::copyToUSB_clicked( bool ) {
//    debug( "+ PrepareTab::copyToUSB_clicked\n" );
//
//    QDir jobDir   { _printJob->jobWorkingDirectory };
//    QDir mediaDir { _usbPath };
//    mediaDir.mkdir( jobDir.dirName( ) );
//
//    QDirIterator it { _printJob->jobWorkingDirectory };
//    while ( it.hasNext( ) ) {
//        QString   fileName { it.next( ) };
//        QFileInfo fileInfo { fileName   };
//
//        QString dest { _usbPath % Slash % jobDir.dirName( ) % Slash % fileInfo.fileName( ) };
//
//        debug( "  + copying %s\n", dest.toUtf8( ).data( ) );
//        if ( !QFile::copy( fileName, dest ) ) {
//            // TODO
//        }
//    }
//
//    update( );
//}

void PrepareTab::usbMountManager_filesystemMounted( QString const& mountPoint ) {
    debug( "+ PrepareTab::usbMountManager_filesystemMounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );

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

    update( );
}

void PrepareTab::usbMountManager_filesystemUnmounted( QString const& mountPoint ) {
    debug( "+ PrepareTab::usbMountManager_filesystemUnmounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );

    if ( mountPoint != _usbPath ) {
        debug( "  + not our filesystem; ignoring\n", _usbPath.toUtf8( ).data( ) );
        return;
    }

    _usbPath.clear( );


    update( );
}
