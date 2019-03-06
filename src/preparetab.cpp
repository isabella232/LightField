#include "pch.h"

#include "preparetab.h"

#include "hasher.h"
#include "printjob.h"
#include "shepherd.h"
#include "strings.h"
#include "svgrenderer.h"
#include "utils.h"

PrepareTab::PrepareTab( QWidget* parent ): QWidget( parent ) {
    _initialShowEventFunc = std::bind( &PrepareTab::_initialShowEvent, this );

    auto origFont = font( );
    auto boldFont = ModifyFont( origFont, origFont.pointSizeF( ), QFont::Bold );
    auto font22pt = ModifyFont( origFont, 22.0 );

    layerThicknessLabel->setText( "Layer height:" );

    layerThickness100Button->setChecked( true );
    layerThickness100Button->setText( "Standard res (100 µm)" );
    QObject::connect( layerThickness100Button, &QPushButton::clicked, this, &PrepareTab::layerThickness100Button_clicked );

    layerThickness50Button->setText( "High res (50 µm)" );
    QObject::connect( layerThickness50Button,  &QPushButton::clicked, this, &PrepareTab::layerThickness50Button_clicked  );

    layerThicknessButtonsLayout->setContentsMargins( { } );
    layerThicknessButtonsLayout->addWidget( layerThickness100Button );
    layerThicknessButtonsLayout->addWidget( layerThickness50Button  );

    sliceStatusLabel->setText( "Slicer status:" );
    sliceStatusLabel->setBuddy( sliceStatus );

    sliceStatus->setText( "idle" );
    sliceStatus->setFont( boldFont );

    imageGeneratorStatusLabel->setText( "Image generator:" );
    imageGeneratorStatusLabel->setBuddy( imageGeneratorStatus );

    imageGeneratorStatus->setText( "idle" );
    imageGeneratorStatus->setFont( boldFont );

    _prepareMessage->setAlignment( Qt::AlignCenter );
    _prepareMessage->setTextFormat( Qt::RichText );
    _prepareMessage->setText( QString( "Tap the <b>Prepare</b> button below<br>to prepare the printer." ) );

    _prepareProgress->setRange( 0, 0 );
    _prepareProgress->hide( );

    _prepareInnerLayout->setContentsMargins( { } );
    _prepareInnerLayout->addStretch( ); _prepareInnerLayout->addWidget( _prepareMessage,  0, Qt::AlignCenter );
    _prepareInnerLayout->addStretch( ); _prepareInnerLayout->addWidget( _prepareProgress, 0, Qt::AlignCenter );
    _prepareInnerLayout->addStretch( );

    _prepareGroup->setTitle( "Printer preparation" );
    _prepareGroup->setLayout( _prepareInnerLayout );

    _prepareButton->setEnabled( true );
    _prepareButton->setFixedSize( MainButtonSize );
    _prepareButton->setFont( font22pt );
    _prepareButton->setText( QString( "Prepare" ) );
    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::_prepareButton_clicked );

    _prepareLayout = WrapWidgetsInVBox( { _prepareGroup, _prepareButton } );

    optionsLayout->setContentsMargins( { } );
    optionsLayout->addWidget( layerThicknessLabel );
    optionsLayout->addLayout( layerThicknessButtonsLayout );
    optionsLayout->addLayout( WrapWidgetsInHBox( { sliceStatusLabel,          nullptr, sliceStatus          } ) );
    optionsLayout->addLayout( WrapWidgetsInHBox( { imageGeneratorStatusLabel, nullptr, imageGeneratorStatus } ) );
    optionsLayout->addLayout( _prepareLayout );

    optionsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    optionsContainer->setLayout( optionsLayout );

    sliceButton->setEnabled( false );
    sliceButton->setFixedSize( MainButtonSize );
    sliceButton->setFont( font22pt );
    sliceButton->setText( "Slice" );
    QObject::connect( sliceButton, &QPushButton::clicked, this, &PrepareTab::sliceButton_clicked );

    currentSliceImage->setAlignment( Qt::AlignCenter );
    currentSliceImage->setContentsMargins( { } );
    currentSliceImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentSliceImage->setStyleSheet( QString( "QWidget { background: black }" ) );

    currentSliceLayout->setAlignment( Qt::AlignCenter );
    currentSliceLayout->setContentsMargins( { } );
    currentSliceLayout->addWidget( currentSliceImage );

    currentSliceGroup->setTitle( "Current layer" );
    currentSliceGroup->setMinimumSize( MaximalRightHandPaneSize );
    currentSliceGroup->setLayout( currentSliceLayout );

    _layout->setContentsMargins( { } );
    _layout->addWidget( optionsContainer,  0, 0, 1, 1 );
    _layout->addWidget( sliceButton,       1, 0, 1, 1 );
    _layout->addWidget( currentSliceGroup, 0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setContentsMargins( { } );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    setLayout( _layout );
}

PrepareTab::~PrepareTab( ) {
    /*empty*/
}

void PrepareTab::showEvent( QShowEvent* ev ) {
    if ( _initialShowEventFunc ) {
        _initialShowEventFunc( );
        _initialShowEventFunc = nullptr;
    }
    ev->ignore( );
}

void PrepareTab::_initialShowEvent( ) {
    currentSliceImage->setFixedWidth( currentSliceImage->width( ) );
    currentSliceImage->setFixedHeight( currentSliceImage->width( ) / AspectRatio16to10 + 0.5 );
}

bool PrepareTab::_checkPreSlicedFiles( ) {
    debug( "+ PrepareTab::_checkPreSlicedFiles\n" );

    // check that the sliced SVG file is newer than the STL file
    auto modelFile     = QFileInfo { _printJob->modelFileName     };
    auto slicedSvgFile = QFileInfo { _printJob->slicedSvgFileName };
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
    debug( "+ PrepareTab::_checkPreSlicedFiles: Success, %d layers\n", _printJob->layerCount );

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

void PrepareTab::sliceButton_clicked( bool ) {
    debug( "+ PrepareTab::sliceButton_clicked: kicking off hasher\n" );

    _hasher = new Hasher;
    QObject::connect( _hasher, &Hasher::resultReady, this, &PrepareTab::hasher_resultReady );
    _hasher->hash( _printJob->modelFileName );

    sliceStatus->setText( "hashing" );
    imageGeneratorStatus->setText( "waiting" );
    currentSliceImage->clear( );
    emit sliceStarted( );
}

void PrepareTab::hasher_resultReady( QString const hash ) {
    debug( "+ PrepareTab::hasher_resultReady:\n  + result hash:           '%s'\n", hash.toUtf8( ).data( ) );

    _printJob->jobWorkingDirectory = JobWorkingDirectoryPath + Slash + ( hash.isEmpty( ) ? QString( "%1-%2" ).arg( time( nullptr ) ).arg( getpid( ) ) : hash );
    _hasher->deleteLater( );
    _hasher = nullptr;

    auto baseName = GetFileBaseName( _printJob->modelFileName );
    _printJob->slicedSvgFileName = _printJob->jobWorkingDirectory + Slash + QString( "sliced.svg" );

    debug(
        "  + model filename:        '%s'\n"
        "  + sliced SVG filename:   '%s'\n"
        "  + job working directory: '%s'\n"
        "",
        _printJob->modelFileName.toUtf8( ).data( ),
        _printJob->slicedSvgFileName.toUtf8( ).data( ),
        _printJob->jobWorkingDirectory.toUtf8( ).data( )
    );

    if ( 0 != ::mkdir( _printJob->jobWorkingDirectory.toUtf8( ).data( ), 0700 ) ) {
        error_t err = errno;
        // if err is EEXIST [17] then it may already be sliced and ready for us, but for now, just ignore that
        if ( EEXIST != err ) {
            debug( "  + unable to create job working directory: %s [%d]\n", strerror( err ), err );
            emit sliceComplete( false );
            return;
        }

        if ( _checkPreSlicedFiles( ) ) {
            sliceStatus->setText( "skipped" );
            imageGeneratorStatus->setText( "skipped" );
            emit sliceComplete( true );
            emit renderStarted( );
            emit renderComplete( true );
            return;
        }
    }

    QDir jobDir { _printJob->jobWorkingDirectory };
    for ( auto entryName : jobDir.entryList( ) ) {
        jobDir.remove( entryName );
    }

    sliceStatus->setText( "starting" );
    slicerProcess = new QProcess( this );
    QObject::connect( slicerProcess, &QProcess::errorOccurred,                                        this, &PrepareTab::slicerProcessErrorOccurred );
    QObject::connect( slicerProcess, &QProcess::started,                                              this, &PrepareTab::slicerProcessStarted       );
    QObject::connect( slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrepareTab::slicerProcessFinished      );
    slicerProcess->start(
        QString     { "slic3r" },
        QStringList {
            _printJob->modelFileName,
            QString( "--export-svg" ),
            QString( "--layer-height" ),
            QString( "%1" ).arg( _printJob->layerThickness / 1000.0 ),
            QString( "--output" ),
            _printJob->slicedSvgFileName
        }
    );
}

void PrepareTab::slicerProcessErrorOccurred( QProcess::ProcessError error ) {
    debug( "+ PrepareTab::slicerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        debug( "  + slicer process failed to start\n" );
        sliceStatus->setText( "failed to start" );
    } else if ( QProcess::Crashed == error ) {
        debug( "  + slicer process crashed? state is %s [%d]\n", ToString( slicerProcess->state( ) ), slicerProcess->state( ) );
        if ( slicerProcess->state( ) != QProcess::NotRunning ) {
            slicerProcess->kill( );
            debug( "  + slicer terminated\n" );
        }
        sliceStatus->setText( "crashed" );
    }
}

void PrepareTab::slicerProcessStarted( ) {
    debug( "+ PrepareTab::slicerProcessStarted\n" );
    sliceStatus->setText( "started" );
}

void PrepareTab::slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    QObject::disconnect( slicerProcess, nullptr, this, nullptr );

    debug( "+ PrepareTab::slicerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    slicerProcess->deleteLater( );
    slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        debug( "  + slicer process crashed?\n" );
        sliceStatus->setText( "crashed" );
        emit sliceComplete( false );
        return;
    } else if ( ( exitStatus == QProcess::NormalExit ) && ( exitCode != 0 ) ) {
        debug( "  + slicer process failed\n" );
        sliceStatus->setText( "failed" );
        emit sliceComplete( false );
        return;
    }

    sliceStatus->setText( "finished" );
    emit sliceComplete( true );

    svgRenderer = new SvgRenderer;
    QObject::connect( svgRenderer, &SvgRenderer::nextLayer, this, &PrepareTab::svgRenderer_progress );
    QObject::connect( svgRenderer, &SvgRenderer::done,      this, &PrepareTab::svgRenderer_done     );
    svgRenderer->startRender( _printJob->slicedSvgFileName, _printJob->jobWorkingDirectory );

    emit renderStarted( );
}

void PrepareTab::svgRenderer_progress( int const currentLayer ) {
    imageGeneratorStatus->setText( QString( "layer %1" ).arg( currentLayer ) );
    if ( 0 != ( currentLayer % 5 ) ) {
        return;
    }

    auto pixmap = QPixmap( _printJob->jobWorkingDirectory + QString( "/%2.png" ).arg( currentLayer - 1, 6, 10, DigitZero ) );
    if ( ( pixmap.width( ) > currentSliceImage->width( ) ) || ( pixmap.height( ) > currentSliceImage->height( ) ) ) {
        pixmap = pixmap.scaled( currentSliceImage->size( ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }
    currentSliceImage->setPixmap( pixmap );
}

void PrepareTab::svgRenderer_done( int const totalLayers ) {
    if ( totalLayers == -1 ) {
        imageGeneratorStatus->setText( QString( "failed" ) );
    } else {
        imageGeneratorStatus->setText( QString( "finished" ) );
        _printJob->layerCount = totalLayers;
    }

    svgRenderer->deleteLater( );
    svgRenderer = nullptr;

    emit renderComplete( totalLayers != -1 );
}

void PrepareTab::_prepareButton_clicked( bool ) {
    debug( "+ PrepareTab::_prepareButton_clicked\n" );

    QObject::disconnect( _prepareButton, nullptr, this, nullptr );

    _prepareMessage->setText( QString( "Moving the printer to<br>its home location..." ) );
    _prepareProgress->show( );

    _prepareButton->setText( QString( "Continue" ) );
    _prepareButton->setEnabled( false );

    QObject::connect( _shepherd, &Shepherd::action_homeComplete, this, &PrepareTab::_shepherd_homeComplete );
    _shepherd->doHome( );
}

void PrepareTab::_shepherd_homeComplete( bool const success ) {
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

    _prepareMessage->setText( QString( "<div style='text-align: center;'>Adjust the build platform position,<br>then tap <b>Continue</b>.</div>" ) );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::_adjustBuildPlatform_complete );
    _prepareButton->setEnabled( true );
}

void PrepareTab::_adjustBuildPlatform_complete( bool ) {
    debug( "+ PrepareTab::_adjustBuildPlatform_complete\n" );

    QObject::disconnect( _prepareButton, nullptr, this, nullptr );
    _prepareButton->setEnabled( false );

    _prepareMessage->setText( QString( "<div style='text-align: center;'>Raising the build platform...</div>" ) );
    _prepareProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_moveToComplete, this, &PrepareTab::_shepherd_resinLoadMoveToComplete );
    _shepherd->doMoveTo( PrinterMaximumZ );
}

void PrepareTab::_shepherd_resinLoadMoveToComplete( bool const success ) {
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

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::_prepareButton_clicked );
    _prepareButton->setText( "Prepare" );
    _prepareButton->setEnabled( true );

    emit preparePrinterComplete( true );
}

void PrepareTab::setPrepareButtonEnabled( bool const value ) {
    _prepareButton->setEnabled( value );
}

void PrepareTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ PrepareTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void PrepareTab::setShepherd( Shepherd* newShepherd ) {
    if ( _shepherd ) {
        QObject::disconnect( _shepherd, nullptr, this, nullptr );
    }

    _shepherd = newShepherd;
}

void PrepareTab::setSliceButtonEnabled( bool const value ) {
    sliceButton->setEnabled( value );
}

void PrepareTab::resetState( ) {
    sliceStatus->setText( "idle" );
    imageGeneratorStatus->setText( "idle" );
    currentSliceImage->clear( );
}
