#include "pch.h"

#include "preparetab.h"

#include "printjob.h"
#include "strings.h"
#include "svgrenderer.h"

namespace {

    std::vector<int> LayerThicknessValues { 50, 100, 200 };

}

PrepareTab::PrepareTab( QWidget* parent ): QWidget( parent ) {
    layerThicknessComboBox->setEditable( false );
    layerThicknessComboBox->setMaxVisibleItems( LayerThicknessValues.size( ) );
    for ( auto value : LayerThicknessValues ) {
        layerThicknessComboBox->addItem( QString( "%1 µm" ).arg( value ) );
    }
    layerThicknessComboBox->setCurrentIndex( 1 );
    QObject::connect( layerThicknessComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &PrepareTab::layerThicknessComboBox_currentIndexChanged );

    layerThicknessLabel->setText( "Layer thickness:" );
    layerThicknessLabel->setBuddy( layerThicknessComboBox );

    sliceProgressLabel->setText( "Slicer status:" );
    sliceProgress->setText( "Not slicing" );
    sliceProgress->setFrameShadow( QFrame::Sunken );
    sliceProgress->setFrameStyle( QFrame::StyledPanel );

    renderProgressLabel->setText( "Render status:" );
    renderProgress->setText( "Not rendering" );
    renderProgress->setFrameShadow( QFrame::Sunken );
    renderProgress->setFrameStyle( QFrame::StyledPanel );

    optionsLayout->setContentsMargins( { } );
    optionsLayout->addWidget( layerThicknessLabel );
    optionsLayout->addWidget( layerThicknessComboBox );
    optionsLayout->addStretch( );
    optionsLayout->addWidget( sliceProgressLabel );
    optionsLayout->addWidget( sliceProgress );
    optionsLayout->addWidget( renderProgressLabel );
    optionsLayout->addWidget( renderProgress );

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

    currentSliceImage->setAlignment( Qt::AlignCenter );
    currentSliceImage->setContentsMargins( { } );
    currentSliceImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    {
        auto pal = currentSliceImage->palette( );
        pal.setColor( QPalette::Background, Qt::black );
        currentSliceImage->setPalette( pal );
    }

    currentSliceLayout->addWidget( currentSliceImage );

    currentSliceGroup->setTitle( QString( "Current slice" ) );
    currentSliceGroup->setMinimumSize( MaximalRightHandPaneSize );
    currentSliceGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentSliceGroup->setLayout( currentSliceLayout );

    _layout = new QGridLayout;
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
        sliceProgress->setText( "Slicer failed to start" );
    } else if ( QProcess::Crashed == error ) {
        debug( "  + slicer process crashed? state is %s [%d]\n", ToString( slicerProcess->state( ) ), slicerProcess->state( ) );
        if ( slicerProcess->state( ) != QProcess::NotRunning ) {
            slicerProcess->kill( );
            debug( "  + slicer terminated\n" );
        }
        sliceProgress->setText( "Slicer crashed" );
    }
}

void PrepareTab::slicerProcessStarted( ) {
    debug( "+ PrepareTab::slicerProcessStarted\n" );
    sliceProgress->setText( "Slicer started" );
    emit sliceStarting( );
}

void PrepareTab::slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    QObject::disconnect( slicerProcess, nullptr, this, nullptr );

    debug( "+ PrepareTab::slicerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    slicerProcess->deleteLater( );
    slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        debug( "  + slicer process crashed?\n" );
        sliceProgress->setText( "Slicer crashed" );
        emit sliceComplete( false );
        return;
    }

    sliceProgress->setText( "Slicer finished" );
    emit sliceComplete( true );

    svgRenderer = new SvgRenderer;
    QObject::connect( svgRenderer, &SvgRenderer::nextLayer, this, &PrepareTab::svgRenderer_progress );
    QObject::connect( svgRenderer, &SvgRenderer::done,      this, &PrepareTab::svgRenderer_done     );
    svgRenderer->startRender( _printJob->slicedSvgFileName, _printJob->pngFilesPath );

    emit renderStarting( );
}

void PrepareTab::svgRenderer_progress( int const currentLayer ) {
    if ( 0 == ( currentLayer % 5 ) ) {
        renderProgress->setText( QString( "Rendering layer %1" ).arg( currentLayer ) );
        if ( currentLayer > 0 ) {
            auto pngFileName = QString( "%1/%2.png" ).arg( _printJob->pngFilesPath ).arg( currentLayer - 1, 6, 10, QChar( '0' ) );
            auto pixMap = QPixmap( pngFileName );
            currentSliceImage->setPixmap( pixMap );
        }
    }
}

void PrepareTab::svgRenderer_done( int const totalLayers ) {
    if ( totalLayers == -1 ) {
        renderProgress->setText( QString( "Rendering failed" ) );
    } else {
        renderProgress->setText( QString( "Rendering complete" ) );
        _printJob->layerCount = totalLayers;
    }

    svgRenderer->deleteLater( );
    svgRenderer = nullptr;

    emit renderComplete( totalLayers != -1 );
}

void PrepareTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ PrepareTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}
