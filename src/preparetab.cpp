#include "pch.h"

#include "preparetab.h"

#include "printjob.h"
#include "shepherd.h"
#include "strings.h"
#include "svgrenderer.h"
#include "utils.h"

PrepareTab::PrepareTab( QWidget* parent ): QWidget( parent ) {
    _initialShowEventFunc = std::bind( &PrepareTab::_initialShowEvent, this );

    auto boldFont = ModifyFont( font( ), font( ).pointSizeF( ), QFont::Bold );
    auto font16pt = ModifyFont( font( ), 16.0f );
    auto font22pt = ModifyFont( font( ), 22.0f );

    layerThicknessLabel->setText( "Layer height:" );

    layerThickness50Button->setText( "High res (50 µm)" );
    layerThickness100Button->setText( "Standard res (100 µm)" );
    layerThickness100Button->setChecked( true );
    QObject::connect( layerThickness50Button,  &QPushButton::clicked, this, &PrepareTab::layerThickness50Button_clicked  );
    QObject::connect( layerThickness100Button, &QPushButton::clicked, this, &PrepareTab::layerThickness100Button_clicked );

    layerThicknessButtonsLayout->setContentsMargins( { } );
    layerThicknessButtonsLayout->addWidget( layerThickness100Button );
    layerThicknessButtonsLayout->addWidget( layerThickness50Button  );


    sliceStatusLabel->setText( "Slicer status:" );
    sliceStatusLabel->setBuddy( sliceStatus );

    sliceStatus->setText( "Idle" );
    sliceStatus->setFrameShadow( QFrame::Sunken );
    sliceStatus->setFrameStyle( QFrame::StyledPanel );
    sliceStatus->setFont( boldFont );

    sliceStatusLayout->addWidget( sliceStatusLabel );
    sliceStatusLayout->addStretch( );
    sliceStatusLayout->addWidget( sliceStatus );


    imageGeneratorStatusLabel->setText( "Image generator:" );
    imageGeneratorStatusLabel->setBuddy( imageGeneratorStatus );

    imageGeneratorStatus->setText( "Idle" );
    imageGeneratorStatus->setFrameShadow( QFrame::Sunken );
    imageGeneratorStatus->setFrameStyle( QFrame::StyledPanel );
    imageGeneratorStatus->setFont( boldFont );

    imageGeneratorStatusLayout->addWidget( imageGeneratorStatusLabel );
    imageGeneratorStatusLayout->addStretch( );
    imageGeneratorStatusLayout->addWidget( imageGeneratorStatus );


    currentSliceLabel->setText( "Current layer:" );
    currentSliceLabel->setBuddy( currentSliceImage );

    currentSliceImage->setAlignment( Qt::AlignCenter );
    currentSliceImage->setContentsMargins( { } );
    currentSliceImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    {
        auto pal = currentSliceImage->palette( );
        pal.setColor( QPalette::Window, Qt::black );
        currentSliceImage->setPalette( pal );
    }

    currentSliceLayout->setContentsMargins( { } );
    currentSliceLayout->setAlignment( Qt::AlignHCenter );
    currentSliceLayout->addWidget( currentSliceImage );

    optionsLayout->setContentsMargins( { } );
    optionsLayout->addWidget( layerThicknessLabel );
    optionsLayout->addLayout( layerThicknessButtonsLayout );
    optionsLayout->addLayout( sliceStatusLayout );
    optionsLayout->addLayout( imageGeneratorStatusLayout );
    optionsLayout->addWidget( currentSliceLabel );
    optionsLayout->addLayout( currentSliceLayout );

    optionsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    optionsContainer->setLayout( optionsLayout );

    sliceButton->setFont( font22pt );
    sliceButton->setText( "Slice" );
    sliceButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    sliceButton->setEnabled( false );
    QObject::connect( sliceButton, &QPushButton::clicked, this, &PrepareTab::sliceButton_clicked );

    _prepareMessage->setFont( font16pt );
    _prepareMessage->setTextFormat( Qt::RichText );
    _prepareMessage->setText( QString( "Tap the <b>Prepare</b> button below<br>to prepare the printer." ) );
    _prepareMessage->setAlignment( Qt::AlignCenter );
    _prepareMessage->setFixedWidth( MaximalRightHandPaneSize.width( ) * 85 / 100 );
    _prepareMessage->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

    _prepareProgress->setRange( 0, 0 );
    _prepareProgress->hide( );

    _prepareButton->setFont( font16pt );
    _prepareButton->setText( QString( "Prepare" ) );
    _prepareButton->setEnabled( true );
    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::_prepareButton_clicked );

    _prepareLayout->setContentsMargins( { } );
    _prepareLayout->addStretch( ); _prepareLayout->addWidget( _prepareMessage,  0, Qt::AlignCenter );
    _prepareLayout->addStretch( ); _prepareLayout->addWidget( _prepareProgress, 0, Qt::AlignCenter );
    _prepareLayout->addStretch( ); _prepareLayout->addWidget( _prepareButton,   0, Qt::AlignCenter );
    _prepareLayout->addStretch( );

    _prepareGroup->setTitle( "Printer preparation" );
    _prepareGroup->setMinimumSize( MaximalRightHandPaneSize );
    _prepareGroup->setLayout( _prepareLayout );

    _layout->setContentsMargins( { } );
    _layout->addWidget( optionsContainer, 0, 0, 1, 1 );
    _layout->addWidget( sliceButton,      1, 0, 1, 1 );
    _layout->addWidget( _prepareGroup,    0, 1, 2, 1 );
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
    sliceButton->setFixedSize( sliceButton->size( ) );

    _maxSliceImageWidth = std::min( currentSliceImage->width( ), currentSliceImage->height( ) );
    currentSliceImage->setMaximumSize( _maxSliceImageWidth, _maxSliceImageWidth );

    QFontMetrics fontMetrics( _prepareButton->font( ) );
    auto bboxContinue = fontMetrics.size( 0, QString( "Continue" ) );
    auto bboxPrepare  = fontMetrics.size( 0, QString( "Prepare"  ) );
    auto bboxRetry    = fontMetrics.size( 0, QString( "Retry"    ) );

    auto minWidth = std::min( { bboxContinue.width( ), bboxPrepare.width( ), bboxRetry.width( ) } );
    auto maxWidth = std::max( { bboxContinue.width( ), bboxPrepare.width( ), bboxRetry.width( ) } );

    _prepareButton->setMinimumWidth( _prepareButton->width( ) + ( maxWidth - minWidth ) );
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
    debug( "+ PrepareTab::sliceButton_clicked\n" );

    _printJob->pngFilesPath = StlModelLibraryPath + QString( "/working_%1" ).arg( static_cast<unsigned long long>( getpid( ) ) * 10000000000ull + static_cast<unsigned long long>( rand( ) ) );
    mkdir( _printJob->pngFilesPath.toUtf8( ).data( ), 0700 );

    QString baseName = getFileBaseName( _printJob->modelFileName );
    _printJob->slicedSvgFileName =
        _printJob->pngFilesPath +
        Slash +
        baseName.left( baseName.length( ) - ( baseName.endsWith( ".stl", Qt::CaseInsensitive ) ? 4 : 0 ) ) +
        QString( ".svg" );

    debug(
        "  + model filename:      '%s'\n"
        "  + sliced SVG filename: '%s'\n"
        "  + PNG files path:      '%s'\n"
        "",
        _printJob->modelFileName.toUtf8( ).data( ),
        _printJob->slicedSvgFileName.toUtf8( ).data( ),
        _printJob->pngFilesPath.toUtf8( ).data( )
    );

    slicerProcess = new QProcess( this );
    QObject::connect( slicerProcess, &QProcess::errorOccurred, this, &PrepareTab::slicerProcessErrorOccurred );
    QObject::connect( slicerProcess, &QProcess::started,       this, &PrepareTab::slicerProcessStarted       );
    QObject::connect( slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrepareTab::slicerProcessFinished );
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
        sliceStatus->setText( "Slicer failed to start" );
    } else if ( QProcess::Crashed == error ) {
        debug( "  + slicer process crashed? state is %s [%d]\n", ToString( slicerProcess->state( ) ), slicerProcess->state( ) );
        if ( slicerProcess->state( ) != QProcess::NotRunning ) {
            slicerProcess->kill( );
            debug( "  + slicer terminated\n" );
        }
        sliceStatus->setText( "Slicer crashed" );
    }
}

void PrepareTab::slicerProcessStarted( ) {
    debug( "+ PrepareTab::slicerProcessStarted\n" );
    sliceStatus->setText( "Slicer started" );
    imageGeneratorStatus->setText( "Waiting for slicer" );
    currentSliceImage->clear( );
    emit sliceStarted( );
}

void PrepareTab::slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    QObject::disconnect( slicerProcess, nullptr, this, nullptr );

    debug( "+ PrepareTab::slicerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    slicerProcess->deleteLater( );
    slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        debug( "  + slicer process crashed?\n" );
        sliceStatus->setText( "Slicer crashed" );
        emit sliceComplete( false );
        return;
    }

    sliceStatus->setText( "Slicer finished" );
    emit sliceComplete( true );

    svgRenderer = new SvgRenderer;
    QObject::connect( svgRenderer, &SvgRenderer::nextLayer, this, &PrepareTab::svgRenderer_progress );
    QObject::connect( svgRenderer, &SvgRenderer::done,      this, &PrepareTab::svgRenderer_done     );
    svgRenderer->startRender( _printJob->slicedSvgFileName, _printJob->pngFilesPath );

    emit renderStarted( );
}

void PrepareTab::svgRenderer_progress( int const currentLayer ) {
    if ( 0 == ( currentLayer % 5 ) ) {
        imageGeneratorStatus->setText( QString( "Generating layer %1" ).arg( currentLayer ) );
        if ( currentLayer > 0 ) {
            auto pixmap = QPixmap( _printJob->pngFilesPath + QString( "/%2.png" ).arg( currentLayer - 1, 6, 10, DigitZero ) );
            // comparing height against width is not an error here -- the slice image widget is square
            if ( ( pixmap.width( ) > _maxSliceImageWidth ) || ( pixmap.height( ) > _maxSliceImageWidth ) ) {
                pixmap = pixmap.scaled( _maxSliceImageWidth, _maxSliceImageWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation );
            }
            currentSliceImage->setPixmap( pixmap );
        }
    }
}

void PrepareTab::svgRenderer_done( int const totalLayers ) {
    if ( totalLayers == -1 ) {
        imageGeneratorStatus->setText( QString( "Image generation failed" ) );
    } else {
        imageGeneratorStatus->setText( QString( "Image generation complete" ) );
        _printJob->layerCount = totalLayers;
    }

    svgRenderer->deleteLater( );
    svgRenderer = nullptr;

    emit renderComplete( totalLayers != -1 );
}

void PrepareTab::_prepareButton_clicked( bool ) {
    debug( "+ PrepareTab::_prepareButton_clicked\n" );

    QObject::disconnect( _prepareButton, nullptr, this, nullptr );

    _prepareMessage->setText( QString( "Moving the printer to its home location..." ) );
    _prepareProgress->show( );

    _prepareButton->setText( QString( "Continue" ) );
    _prepareButton->setEnabled( false );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrepareTab::_sendHome_complete );
    _shepherd->doSend( "G28 X" );
}

void PrepareTab::_sendHome_complete( bool const success ) {
    debug( "+ PrepareTab::_sendHome_complete: success: %s\n", success ? "true" : "false" );

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

    _prepareMessage->setText( QString( "<div style='text-align: center;'>Raising the build platform<br>out of the way...</div>" ) );
    _prepareProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrepareTab::_sendResinLoadMove_complete );
    _shepherd->doSend( QStringList {
        QString( "G90" ),
        QString( "G0 X%1" ).arg( PrinterMaximumZ, 0, 'f', 3 )
    } );
}

void PrepareTab::_sendResinLoadMove_complete( bool const success ) {
    debug( "+ PrepareTab::_sendResinLoadMove_complete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    if ( !success ) {
        _prepareMessage->setText( QString( "Preparation failed." ) );

        _prepareButton->setText( QString( "Retry" ) );
        _prepareButton->setEnabled( true );

        emit preparePrinterComplete( false );
        return;
    }

    _prepareMessage->setText( QString( "<div style='text-align: center;'>Load the print solution,<br>then tap <b>Continue</b>.</div>" ) );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::_loadPrintSolution_complete );
    _prepareButton->setEnabled( true );
}

void PrepareTab::_loadPrintSolution_complete( bool ) {
    debug( "+ PrepareTab::_loadPrintSolution_complete\n" );

    QObject::disconnect( _prepareButton, nullptr, this, nullptr );
    _prepareButton->setEnabled( false );

    _prepareMessage->setText( QString( "Lowering the build platform..." ) );
    _prepareProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrepareTab::_sendExtend_complete );
    _shepherd->doSend( QString( "G0 X%1" ).arg( std::max( 100, _printJob->layerThickness ) / 1000.0, 0, 'f', 3 ) );
}

void PrepareTab::_sendExtend_complete( bool const success ) {
    debug( "+ PrepareTab::_sendExtend_complete: success: %s\n", success ? "true" : "false" );

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
    // TODO disconnect/reconnect events?
    _shepherd = newShepherd;
}

void PrepareTab::setSliceButtonEnabled( bool const value ) {
    sliceButton->setEnabled( value );
}
