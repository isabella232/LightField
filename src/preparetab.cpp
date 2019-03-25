#include "pch.h"

#include "preparetab.h"

#include "hasher.h"
#include "printjob.h"
#include "shepherd.h"
#include "strings.h"
#include "svgrenderer.h"
#include "utils.h"

PrepareTab::PrepareTab( QWidget* parent ): TabBase( parent ) {
    _initialShowEventFunc = std::bind( &PrepareTab::_initialShowEvent, this, _1 );

    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto font12pt    = ModifyFont( origFont, 12.0 );
    auto font22pt    = ModifyFont( origFont, 22.0 );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome" );

    _layerThicknessLabel->setText( "Layer height:" );

    _layerThickness100Button->setChecked( true );
    _layerThickness100Button->setFont( font12pt );
    _layerThickness100Button->setText( "Standard res (100 µm)" );
    QObject::connect( _layerThickness100Button, &QPushButton::clicked, this, &PrepareTab::layerThickness100Button_clicked );

    _layerThickness50Button->setText( "High res (50 µm)" );
    _layerThickness50Button->setFont( font12pt );
    QObject::connect( _layerThickness50Button, &QPushButton::clicked, this, &PrepareTab::layerThickness50Button_clicked );

    _layerThicknessButtonsLayout->setContentsMargins( { } );
    _layerThicknessButtonsLayout->addWidget( _layerThickness100Button );
    _layerThicknessButtonsLayout->addWidget( _layerThickness50Button  );

    _sliceStatusLabel->setText( "Slicer:" );
    _sliceStatusLabel->setBuddy( _sliceStatus );

    _sliceStatus->setText( "idle" );
    _sliceStatus->setFont( boldFont );

    _imageGeneratorStatusLabel->setText( "Image generator:" );
    _imageGeneratorStatusLabel->setBuddy( _imageGeneratorStatus );

    _imageGeneratorStatus->setText( "idle" );
    _imageGeneratorStatus->setFont( boldFont );

    _prepareMessage->setAlignment( Qt::AlignCenter );
    _prepareMessage->setTextFormat( Qt::RichText );
    _prepareMessage->setWordWrap( true );
    _prepareMessage->setText( QString( "Tap the <b>Prepare</b> button to prepare the printer." ) );

    _prepareProgress->setAlignment( Qt::AlignCenter );
    _prepareProgress->setFormat( QString( ) );
    _prepareProgress->setRange( 0, 0 );
    _prepareProgress->setTextVisible( false );
    _prepareProgress->hide( );

    _prepareLayout->addStretch( ); _prepareLayout->addLayout( WrapWidgetsInHBox( { _prepareMessage  } ), 1 );
    _prepareLayout->addStretch( ); _prepareLayout->addLayout( WrapWidgetsInHBox( { _prepareProgress } ), 1 );
    _prepareLayout->addStretch( );

    _prepareGroup->setTitle( "Printer preparation" );
    _prepareGroup->setLayout( _prepareLayout );

    _prepareButton->setEnabled( true );
    _prepareButton->setFixedSize( MainButtonSize );
    _prepareButton->setFont( font22pt );
    _prepareButton->setText( QString( "Prepare" ) );
    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );

    _optionsLayout->setContentsMargins( { } );
    _optionsLayout->addWidget( _layerThicknessLabel );
    _optionsLayout->addLayout( _layerThicknessButtonsLayout );
    _optionsLayout->addLayout( WrapWidgetsInHBox( { _sliceStatusLabel,          nullptr, _sliceStatus          } ) );
    _optionsLayout->addLayout( WrapWidgetsInHBox( { _imageGeneratorStatusLabel, nullptr, _imageGeneratorStatus } ) );
    _optionsLayout->addLayout( WrapWidgetsInVBox( { _prepareGroup, _prepareButton } ) );

    _optionsContainer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _optionsContainer->setLayout( _optionsLayout );
    _optionsContainer->setFixedWidth( MainButtonSize.width( ) );

    _sliceButton->setEnabled( false );
    _sliceButton->setFixedSize( MainButtonSize );
    _sliceButton->setFont( font22pt );
    _sliceButton->setText( "Slice" );
    QObject::connect( _sliceButton, &QPushButton::clicked, this, &PrepareTab::sliceButton_clicked );

    _currentSliceImage->setAlignment( Qt::AlignCenter );
    _currentSliceImage->setContentsMargins( { } );
    _currentSliceImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentSliceImage->setStyleSheet( QString( "QWidget { background: black }" ) );

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

    _navigationLayout = WrapWidgetsInHBox( { nullptr, _navigateFirst, _navigatePrevious, _navigateCurrentLabel, _navigateNext, _navigateLast, nullptr } );
    _navigationLayout->setAlignment( Qt::AlignCenter );

    _setNavigationButtonsEnabled( false );

    _currentSliceLayout->setAlignment( Qt::AlignTop | Qt::AlignHCenter );
    _currentSliceLayout->setContentsMargins( { } );
    _currentSliceLayout->addWidget( _currentSliceImage );
    _currentSliceLayout->addLayout( _navigationLayout );

    _currentSliceGroup->setTitle( "Current layer" );
    _currentSliceGroup->setMinimumSize( MaximalRightHandPaneSize );
    _currentSliceGroup->setLayout( _currentSliceLayout );

    _layout->setContentsMargins( { } );
    _layout->addWidget( _optionsContainer,  0, 0, 1, 1 );
    _layout->addWidget( _sliceButton,       1, 0, 1, 1 );
    _layout->addWidget( _currentSliceGroup, 0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setContentsMargins( { } );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    setLayout( _layout );
}

PrepareTab::~PrepareTab( ) {
    /*empty*/
}

void PrepareTab::_initialShowEvent( QShowEvent* event ) {
    _currentSliceImage->setFixedWidth( _currentSliceImage->width( ) );
    _currentSliceImage->setFixedHeight( _currentSliceImage->width( ) / AspectRatio16to10 + 0.5 );
    event->accept( );
}

bool PrepareTab::_checkPreSlicedFiles( ) {
    debug( "+ PrepareTab::_checkPreSlicedFiles\n" );

    // check that the sliced SVG file is newer than the STL file
    auto modelFile     = QFileInfo { _printJob->modelFileName                                   };
    auto slicedSvgFile = QFileInfo { _printJob->jobWorkingDirectory + Slash + SlicedSvgFileName };
    if ( !modelFile.exists( ) ) {
        debug( "+ PrepareTab::_checkPreSlicedFiles: Fail: model file does not exist\n" );
        return false;
    }
    if ( !slicedSvgFile.exists( ) ) {
        debug( "+ PrepareTab::_checkPreSlicedFiles: Fail: sliced SVG file does not exist\n" );
        return false;
    }

    auto slicedSvgFileLastModified = slicedSvgFile.lastModified( );
    if ( modelFile.lastModified( ) > slicedSvgFileLastModified ) {
        debug( "+ PrepareTab::_checkPreSlicedFiles: Fail: model file is newer than sliced SVG file\n" );
        return false;
    }

    int layerNumber     = -1;
    int prevLayerNumber = -1;

    auto jobDir = QDir { _printJob->jobWorkingDirectory };
    jobDir.setSorting( QDir::Name );
    jobDir.setNameFilters( { "[0-9]?????.svg" } );

    // check that the layer SVG files are newer than the sliced SVG file,
    //   and that the layer PNG files are newer than the layer SVG files,
    //   and that there are no gaps in the numbering.
    for ( auto entry : jobDir.entryInfoList( ) ) {
        if ( slicedSvgFileLastModified > entry.lastModified( ) ) {
            debug( "+ PrepareTab::_checkPreSlicedFiles: Fail: sliced SVG file is newer than layer SVG file %s\n", entry.fileName( ).toUtf8( ).data( ) );
            return false;
        }

        auto layerPngFile = QFileInfo { entry.path( ) + Slash + entry.completeBaseName( ) + QString( ".png" ) };
        if ( !layerPngFile.exists( ) ) {
            debug( "+ PrepareTab::_checkPreSlicedFiles: Fail: layer PNG file %s does not exist\n", layerPngFile.fileName( ).toUtf8( ).data( ) );
            return false;
        }
        if ( entry.lastModified( ) > layerPngFile.lastModified( ) ) {
            debug( "+ PrepareTab::_checkPreSlicedFiles: Fail: layer SVG file %s is newer than layer PNG file %s\n", entry.fileName( ).toUtf8( ).data( ), layerPngFile.fileName( ).toUtf8( ).data( ) );
            return false;
        }

        layerNumber = RemoveFileExtension( entry.baseName( ) ).toInt( );
        if ( layerNumber != ( prevLayerNumber + 1 ) ) {
            debug( "+ PrepareTab::_checkPreSlicedFiles: Fail: gap in layer numbers between %d and %d\n", prevLayerNumber, layerNumber );
            return false;
        }
        prevLayerNumber = layerNumber;
    }

    _printJob->layerCount = layerNumber + 1;
    debug( "+ PrepareTab::_checkPreSlicedFiles: Success: %d layers\n", _printJob->layerCount );

    return true;
}

void PrepareTab::layerThickness50Button_clicked( bool checked ) {
    debug( "+ PrepareTab::layerThickness50Button_clicked\n" );
    _printJob->layerThickness = 50;
}

void PrepareTab::layerThickness100Button_clicked( bool checked ) {
    debug( "+ PrepareTab::layerThickness100Button_clicked\n" );
    _printJob->layerThickness = 100;
}

void PrepareTab::_setNavigationButtonsEnabled( bool const enabled ) {
    _navigateFirst   ->setEnabled( enabled && ( _visibleLayer > 0 ) );
    _navigatePrevious->setEnabled( enabled && ( _visibleLayer > 0 ) );
    _navigateNext    ->setEnabled( enabled && ( _printJob && ( _visibleLayer + 1 < _printJob->layerCount ) ) );
    _navigateLast    ->setEnabled( enabled && ( _printJob && ( _visibleLayer + 1 < _printJob->layerCount ) ) );
}

void PrepareTab::_showLayerImage( int const layer ) {
    auto pixmap = QPixmap( _printJob->jobWorkingDirectory + QString( "/%2.png" ).arg( layer, 6, 10, DigitZero ) );
    if ( ( pixmap.width( ) > _currentSliceImage->width( ) ) || ( pixmap.height( ) > _currentSliceImage->height( ) ) ) {
        pixmap = pixmap.scaled( _currentSliceImage->size( ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }
    _currentSliceImage->setPixmap( pixmap );

    int fieldWidth = ceil( log10( _printJob->layerCount ) );
    _navigateCurrentLabel->setText( QString( "%1/%2" ).arg( layer + 1, fieldWidth, 10, FigureSpace ).arg( _printJob->layerCount ) );
}

void PrepareTab::navigateFirst_clicked( bool ) {
    _visibleLayer = 0;
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );
}

void PrepareTab::navigatePrevious_clicked( bool ) {
    if ( _visibleLayer > 0 ) {
        --_visibleLayer;
    }
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );
}

void PrepareTab::navigateNext_clicked( bool ) {
    if ( _visibleLayer + 1 < _printJob->layerCount ) {
        ++_visibleLayer;
    }
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );
}

void PrepareTab::navigateLast_clicked( bool ) {
    _visibleLayer = _printJob->layerCount - 1;
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );
}

void PrepareTab::sliceButton_clicked( bool ) {
    debug( "+ PrepareTab::sliceButton_clicked\n" );

    QDir jobDir { _printJob->jobWorkingDirectory };
    jobDir.removeRecursively( );
    jobDir.mkdir( _printJob->jobWorkingDirectory );

    _sliceStatus->setText( "starting" );
    _imageGeneratorStatus->setText( "waiting" );

    _slicerProcess = new QProcess( this );
    QObject::connect( _slicerProcess, &QProcess::errorOccurred,                                        this, &PrepareTab::slicerProcessErrorOccurred );
    QObject::connect( _slicerProcess, &QProcess::started,                                              this, &PrepareTab::slicerProcessStarted       );
    QObject::connect( _slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrepareTab::slicerProcessFinished      );
    _slicerProcess->start(
        QString     { "slic3r" },
        QStringList {
            _printJob->modelFileName,
            QString( "--export-svg" ),
            QString( "--first-layer-height" ), QString( "%1" ).arg( _printJob->layerThickness / 1000.0 ),
            QString( "--layer-height" ),       QString( "%1" ).arg( _printJob->layerThickness / 1000.0 ),
            QString( "--output" ),             _printJob->jobWorkingDirectory + Slash + SlicedSvgFileName
        }
    );

    emit sliceStarted( );
}

void PrepareTab::hasher_resultReady( QString const hash ) {
    debug(
        "+ PrepareTab::hasher_resultReady:\n"
        "  + result hash:           '%s'\n"
        "",
        hash.toUtf8( ).data( )
    );

    _printJob->jobWorkingDirectory = JobWorkingDirectoryPath + Slash + ( hash.isEmpty( ) ? QString( "%1-%2" ).arg( time( nullptr ) ).arg( getpid( ) ) : hash ) + QString( "-%1" ).arg( _printJob->layerThickness );
    _hasher->deleteLater( );
    _hasher = nullptr;

    _sliceStatus->setText( "idle" );

    debug(
        "  + model filename:        '%s'\n"
        "  + job working directory: '%s'\n"
        "",
        _printJob->modelFileName.toUtf8( ).data( ),
        _printJob->jobWorkingDirectory.toUtf8( ).data( )
    );

    QDir jobDir { _printJob->jobWorkingDirectory };
    bool preSliced;

    if ( jobDir.exists( ) ) {
        debug( "  + job directory already exists, checking sliced model\n" );
        preSliced = _checkPreSlicedFiles( );
        if ( preSliced ) {
            debug( "  + pre-sliced model is good\n" );
        } else {
            debug( "  + pre-sliced model is NOT good\n" );
            jobDir.removeRecursively( );
        }
    } else {
        debug( "  + job directory does not exist\n" );
        preSliced = false;
    }

    _setNavigationButtonsEnabled( preSliced );
    _sliceButton->setEnabled( true );
    if ( preSliced ) {
        _navigateCurrentLabel->setText( QString( "%1/%2" ).arg( 0, ceil( log10( _printJob->layerCount ) ), 10, FigureSpace ).arg( _printJob->layerCount ) );
        _sliceButton->setText( "Reslice" );
        emit alreadySliced( );
    } else {
        _navigateCurrentLabel->setText( "0/0" );
        _sliceButton->setText( "Slice" );
    }
}

void PrepareTab::slicerProcessErrorOccurred( QProcess::ProcessError error ) {
    debug( "+ PrepareTab::slicerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

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
}

void PrepareTab::slicerProcessStarted( ) {
    debug( "+ PrepareTab::slicerProcessStarted\n" );
    _sliceStatus->setText( "started" );
}

void PrepareTab::slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    QObject::disconnect( _slicerProcess, nullptr, this, nullptr );

    debug( "+ PrepareTab::slicerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    _slicerProcess->deleteLater( );
    _slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        debug( "  + slicer process crashed?\n" );
        _sliceStatus->setText( "crashed" );
        emit sliceComplete( false );
        return;
    } else if ( ( exitStatus == QProcess::NormalExit ) && ( exitCode != 0 ) ) {
        debug( "  + slicer process failed\n" );
        _sliceStatus->setText( "failed" );
        emit sliceComplete( false );
        return;
    }

    _sliceStatus->setText( "finished" );

    emit sliceComplete( true );

    _svgRenderer = new SvgRenderer;
    QObject::connect( _svgRenderer, &SvgRenderer::layerCount,    this, &PrepareTab::svgRenderer_layerCount    );
    QObject::connect( _svgRenderer, &SvgRenderer::layerComplete, this, &PrepareTab::svgRenderer_layerComplete );
    QObject::connect( _svgRenderer, &SvgRenderer::done,          this, &PrepareTab::svgRenderer_done          );
    _svgRenderer->startRender( _printJob->jobWorkingDirectory + Slash + SlicedSvgFileName, _printJob->jobWorkingDirectory );

    emit renderStarted( );
}

void PrepareTab::svgRenderer_layerCount( int const totalLayers ) {
    debug( "+ PrepareTab::svgRenderer_layerCount: totalLayers %d\n", totalLayers );
    _printJob->layerCount = totalLayers;

    int fieldWidth = ceil( log10( _printJob->layerCount ) );
    _navigateCurrentLabel->setText( QString( "%1/%2" ).arg( 0, fieldWidth, 10, FigureSpace ).arg( _printJob->layerCount ) );
}

void PrepareTab::svgRenderer_layerComplete( int const currentLayer ) {
    _renderedLayers = currentLayer;
    _imageGeneratorStatus->setText( QString( "layer %1" ).arg( currentLayer + 1 ) );

    if ( 0 == ( currentLayer % 5 ) ) {
        _visibleLayer = currentLayer;
        _showLayerImage( _visibleLayer );
    }
}

void PrepareTab::svgRenderer_done( bool const success ) {
    _imageGeneratorStatus->setText( success ? "finished" : "failed" );

    _svgRenderer->deleteLater( );
    _svgRenderer = nullptr;

    _setNavigationButtonsEnabled( true );
    _sliceButton->setText( "Reslice" );

    emit renderComplete( success );
}

void PrepareTab::prepareButton_clicked( bool ) {
    debug( "+ PrepareTab::_prepareButton_clicked\n" );

    QObject::disconnect( _prepareButton, nullptr, this, nullptr );

    _prepareMessage->setText( QString( "Moving the printer to its home location..." ) );
    _prepareProgress->show( );

    _prepareButton->setText( QString( "Continue" ) );
    _prepareButton->setEnabled( false );

    QObject::connect( _shepherd, &Shepherd::action_homeComplete, this, &PrepareTab::shepherd_homeComplete );
    _shepherd->doHome( );

    emit preparePrinterStarted( );
}

void PrepareTab::shepherd_homeComplete( bool const success ) {
    debug( "+ PrepareTab::_shepherd_homeComplete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    if ( !success ) {
        _prepareMessage->setText( QString( "Preparation failed." ) );

        _prepareButton->setText( QString( "Retry" ) );
        _prepareButton->setEnabled( true );

        emit preparePrinterComplete( false );
        return;
    }

    _prepareMessage->setText( QString( "Adjust the build platform position, then tap <b>Continue</b>." ) );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::adjustBuildPlatform_complete );
    _prepareButton->setEnabled( true );
}

void PrepareTab::adjustBuildPlatform_complete( bool ) {
    debug( "+ PrepareTab::_adjustBuildPlatform_complete\n" );

    QObject::disconnect( _prepareButton, nullptr, this, nullptr );
    _prepareButton->setEnabled( false );

    _prepareMessage->setText( QString( "Raising the build platform..." ) );
    _prepareProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrepareTab::shepherd_resinLoadMoveToComplete );
    _shepherd->doMoveAbsolute( PrinterMaximumZ );
}

void PrepareTab::shepherd_resinLoadMoveToComplete( bool const success ) {
    debug( "+ PrepareTab::_shepherd_resinLoadMoveToComplete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    if ( !success ) {
        _prepareMessage->setText( QString( "Preparation failed." ) );

        _prepareButton->setText( QString( "Retry" ) );
        _prepareButton->setEnabled( true );

        emit preparePrinterComplete( false );
        return;
    }

    _prepareMessage->setText( QString( "Preparation completed." ) );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );
    _prepareButton->setText( "Prepare" );
    _prepareButton->setEnabled( true );

    emit preparePrinterComplete( true );
}

void PrepareTab::setPrepareButtonEnabled( bool const enabled ) {
    _prepareButton->setEnabled( enabled );
}

void PrepareTab::setSliceButtonEnabled( bool const enabled ) {
    _sliceButton->setEnabled( enabled );
}

void PrepareTab::resetState( ) {
    _sliceStatus->setText( "idle" );
    _imageGeneratorStatus->setText( "idle" );
    _currentSliceImage->clear( );
    _navigateCurrentLabel->setText( "0/0" );
    _setNavigationButtonsEnabled( false );
}

void PrepareTab::modelSelected( ) {
    resetState( );
    _sliceButton->setEnabled( false );

    _hasher = new Hasher;
    QObject::connect( _hasher, &Hasher::resultReady, this, &PrepareTab::hasher_resultReady );
    _hasher->hash( _printJob->modelFileName );
}
