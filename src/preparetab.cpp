#include "pch.h"

#include "preparetab.h"

#include "printjob.h"
#include "shepherd.h"
#include "strings.h"
#include "svgrenderer.h"

namespace {

    std::vector<int> LayerThicknessValues { 50, 100, 200 };

}

PrepareTab::PrepareTab( QWidget* parent ): QWidget( parent ) {
    _initialShowEventFunc = [this] ( ) {
        sliceButton->setFixedSize( sliceButton->size( ) );
        auto maxCoord = std::min( currentSliceImage->width( ), currentSliceImage->height( ) );
        currentSliceImage->setMaximumSize( maxCoord, maxCoord );
        currentSliceImage->setScaledContents( true );

        QFontMetrics fontMetrics( _continueButton->font( ) );
        auto bboxContinue = fontMetrics.size( 0, QString( "Continue" ) );
        auto bboxStart    = fontMetrics.size( 0, QString( "Start"    ) );
        _continueButton->setMinimumWidth( _continueButton->width( ) + bboxContinue.width( ) - bboxStart.width( ) );
    };

    layerThicknessLabel->setText( "Layer thickness:" );
    layerThicknessLabel->setBuddy( layerThicknessComboBox );

    layerThicknessComboBox->setEditable( false );
    layerThicknessComboBox->setMaxVisibleItems( LayerThicknessValues.size( ) );
    for ( auto value : LayerThicknessValues ) {
        layerThicknessComboBox->addItem( QString( "%1 µm" ).arg( value ) );
    }
    layerThicknessComboBox->setCurrentIndex( 1 );
    QObject::connect( layerThicknessComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &PrepareTab::layerThicknessComboBox_currentIndexChanged );

    sliceProgressLabel->setText( "Slicer status:" );
    sliceProgressLabel->setBuddy( sliceStatus );

    sliceStatus->setText( "Not slicing" );
    sliceStatus->setFrameShadow( QFrame::Sunken );
    sliceStatus->setFrameStyle( QFrame::StyledPanel );

    renderProgressLabel->setText( "Render status:" );
    renderProgressLabel->setBuddy( renderStatus );

    renderStatus->setText( "Not rendering" );
    renderStatus->setFrameShadow( QFrame::Sunken );
    renderStatus->setFrameStyle( QFrame::StyledPanel );

    currentSliceLabel->setText( "Current slice:" );
    currentSliceLabel->setBuddy( currentSliceImage );

    currentSliceImage->setAlignment( Qt::AlignCenter );
    currentSliceImage->setContentsMargins( { } );
    currentSliceImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    {
        auto pal = currentSliceImage->palette( );
        pal.setColor( QPalette::Background, Qt::black );
        currentSliceImage->setPalette( pal );
    }

    currentSliceLayout->setContentsMargins( { } );
    currentSliceLayout->setAlignment( Qt::AlignHCenter );
    currentSliceLayout->addWidget( currentSliceImage );

    optionsLayout->setContentsMargins( { } );
    optionsLayout->addWidget( layerThicknessLabel );
    optionsLayout->addWidget( layerThicknessComboBox );
    optionsLayout->addWidget( sliceProgressLabel );
    optionsLayout->addWidget( sliceStatus );
    optionsLayout->addWidget( renderProgressLabel );
    optionsLayout->addWidget( renderStatus );
    optionsLayout->addWidget( currentSliceLabel );
    optionsLayout->addLayout( currentSliceLayout );

    optionsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    optionsContainer->setLayout( optionsLayout );

    {
        auto font { sliceButton->font( ) };
        font.setPointSizeF( 22.25 );
        sliceButton->setFont( font );
    }
    sliceButton->setText( "Slice" );
    sliceButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    sliceButton->setEnabled( false );
    QObject::connect( sliceButton, &QPushButton::clicked, this, &PrepareTab::sliceButton_clicked );

    {
        auto font { _prepareMessage->font( ) };
        font.setPointSizeF( 16.25 );
        _prepareMessage->setFont( font );
    }
    _prepareMessage->setTextFormat( Qt::RichText );
    _prepareMessage->setText( QString( "Tap the <b>Start</b> button below<br>to prepare the printer." ) );
    _prepareMessage->setAlignment( Qt::AlignCenter );
    _prepareMessage->setFixedWidth( MaximalRightHandPaneSize.width( ) * 3 / 4 );
    _prepareMessage->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

    _prepareProgress->setRange( 0, 0 );
    _prepareProgress->hide( );

    {
        auto font { _continueButton->font( ) };
        font.setPointSizeF( 16.25 );
        _continueButton->setFont( font );
    }
    _continueButton->setText( QString( "Start" ) );
    _continueButton->setEnabled( true );
    QObject::connect( _continueButton, &QPushButton::clicked, this, &PrepareTab::_continueButton_clicked );

    _prepareLayout->setContentsMargins( { } );
    _prepareLayout->addStretch( ); _prepareLayout->addWidget( _prepareMessage,  0, Qt::AlignCenter );
    _prepareLayout->addStretch( ); _prepareLayout->addWidget( _prepareProgress, 0, Qt::AlignCenter );
    _prepareLayout->addStretch( ); _prepareLayout->addWidget( _continueButton,  0, Qt::AlignCenter );
    _prepareLayout->addStretch( );

    _prepareGroup->setTitle( "Printer Preparation" );
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

void PrepareTab::layerThicknessComboBox_currentIndexChanged( int index ) {
    debug( "+ PrepareTab::layerThicknessComboBox_currentIndexChanged: new value: %d µm\n", LayerThicknessValues[index] );

    _printJob->layerThickness = LayerThicknessValues[index];
}

void PrepareTab::sliceButton_clicked( bool ) {
    debug( "+ PrepareTab::sliceButton_clicked\n" );

    _printJob->pngFilesPath = StlModelLibraryPath + QString( "/working_%1" ).arg( static_cast<unsigned long long>( getpid( ) ) * 10000000000ull + static_cast<unsigned long long>( rand( ) ) );
    mkdir( _printJob->pngFilesPath.toUtf8( ).data( ), 0700 );

    QString baseName = getFileBaseName( _printJob->modelFileName );
    _printJob->slicedSvgFileName =
        _printJob->pngFilesPath +
        QChar( '/' ) +
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
        renderStatus->setText( QString( "Rendering layer %1" ).arg( currentLayer ) );
        if ( currentLayer > 0 ) {
            auto pngFileName = QString( "%1/%2.png" ).arg( _printJob->pngFilesPath ).arg( currentLayer - 1, 6, 10, QChar( '0' ) );
            auto pixMap = QPixmap( pngFileName );
            currentSliceImage->setPixmap( pixMap );
            debug(
                "+ PrepareTab::svgRenderer_progress:\n"
                "  + current layer:          %d\n"
                "  + pngFileName:            %s\n"
                "  + PNG size:               %d×%d\n"
                "  + currentSliceImage size: %d×%d\n"
                "",
                currentLayer,
                pngFileName.toUtf8( ).data( ),
                pixMap.width( ), pixMap.height( ),
                currentSliceImage->width( ), currentSliceImage->height( )
            );
        }
    }
}

void PrepareTab::svgRenderer_done( int const totalLayers ) {
    if ( totalLayers == -1 ) {
        renderStatus->setText( QString( "Rendering failed" ) );
    } else {
        renderStatus->setText( QString( "Rendering complete" ) );
        _printJob->layerCount = totalLayers;
    }

    svgRenderer->deleteLater( );
    svgRenderer = nullptr;

    emit renderComplete( totalLayers != -1 );
}

void PrepareTab::_continueButton_clicked( bool ) {
    debug( "+ PrepareTab::_continueButton_clicked\n" );

    _prepareMessage->setText( QString( "Moving the printer to its home location..." ) );
    _prepareProgress->show( );

    _continueButton->setText( QString( "Continue" ) );
    _continueButton->setEnabled( false );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrepareTab::_sendHome_complete );
    _shepherd->doSend( "G28 X" );
}

void PrepareTab::_sendHome_complete( bool const success ) {
    debug( "+ PrepareTab::_sendHome_complete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    if ( !success ) {
        _prepareMessage->setText( QString( "Preparation failed." ) );

        _continueButton->setText( QString( "Retry" ) );
        _continueButton->setEnabled( true );

        emit prepareComplete( false );
        return;
    }

    _prepareMessage->setText( QString( "<div style='text-align: center;'>Adjust the build platform position,<br>then tap <b>Continue</b>.</div>" ) );

    QObject::connect( _continueButton, &QPushButton::clicked, this, &PrepareTab::_adjustBuildPlatform_complete );
    _continueButton->setEnabled( true );
}

void PrepareTab::_adjustBuildPlatform_complete( bool ) {
    debug( "+ PrepareTab::_adjustBuildPlatform_complete\n" );

    QObject::disconnect( _continueButton, nullptr, this, nullptr );
    _continueButton->setEnabled( false );

    _prepareMessage->setText( QString( "<div style='text-align: center;'>Raising the build platform<br>out of the way...</div>" ) );
    _prepareProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrepareTab::_sendResinLoadMove_complete );
    _shepherd->doSend( QStringList {
        QString( "G90" ),
        QString( "G0 X%1" ).arg( PrinterMaximumHeight, 0, 'f', 3 )
    } );
}

void PrepareTab::_sendResinLoadMove_complete( bool const success ) {
    debug( "+ PrepareTab::_sendResinLoadMove_complete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    if ( !success ) {
        _prepareMessage->setText( QString( "Preparation failed." ) );

        _continueButton->setText( QString( "Retry" ) );
        _continueButton->setEnabled( true );

        emit prepareComplete( false );
        return;
    }

    _prepareMessage->setText( QString( "<div style='text-align: center;'>Load the print solution,<br>then tap <b>Continue</b>.</div>" ) );

    QObject::connect( _continueButton, &QPushButton::clicked, this, &PrepareTab::_loadPrintSolution_complete );
    _continueButton->setEnabled( true );
}

void PrepareTab::_loadPrintSolution_complete( bool ) {
    debug( "+ PrepareTab::_loadPrintSolution_complete\n" );

    QObject::disconnect( _continueButton, nullptr, this, nullptr );
    _continueButton->setEnabled( false );

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

        _continueButton->setText( QString( "Retry" ) );
        _continueButton->setEnabled( true );

        emit prepareComplete( false );
        return;
    }

    _prepareMessage->setText( QString( "Preparation completed." ) );

    emit prepareComplete( true );
}

void PrepareTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ PrepareTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void PrepareTab::setShepherd( Shepherd* shepherd ) {
    _shepherd = shepherd;
}

void PrepareTab::setSliceButtonEnabled( bool const value ) {
    sliceButton->setEnabled( value );
}
