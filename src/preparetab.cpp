#include "pch.h"

#include "preparetab.h"
#include "svgrenderer.h"

#include "printjob.h"
#include "strings.h"
#include "constants.h"
#include "debug.h"

namespace {

    QStringList LayerThicknessStringList { "50 µm", "100 µm", "200 µm" };
    int         LayerThicknessValues[]   { 50, 100, 200                };
    QString     SlicerCommand            { "slic3r"                    };

}

PrepareTab::PrepareTab( QWidget* parent ): QWidget( parent ) {
    sliceProgress->setText( "Not slicing" );
    renderProgress->setText( "Not rendering" );

    sliceProgressLayout->setContentsMargins( { } );
    sliceProgressLayout->addRow( "Slicer status:", sliceProgress  );
    sliceProgressLayout->addRow( "Render status:", renderProgress );

    sliceProgressContainer->setLayout( sliceProgressLayout );
    sliceProgressContainer->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    layerThicknessComboBox->setEditable( false );
    layerThicknessComboBox->setMaxVisibleItems( LayerThicknessStringList.count( ) );
    layerThicknessComboBox->addItems( LayerThicknessStringList );
    layerThicknessComboBox->setCurrentIndex( 1 );
    QObject::connect( layerThicknessComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &PrepareTab::layerThicknessComboBox_currentIndexChanged );

    layerThicknessLabel->setText( "Layer thickness:" );
    layerThicknessLabel->setBuddy( layerThicknessComboBox );

    optionsLayout->setContentsMargins( { } );
    optionsLayout->addWidget( layerThicknessLabel );
    optionsLayout->addWidget( layerThicknessComboBox );
    optionsLayout->addStretch( );

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

    currentSliceLabel->setText( "Current slice:" );
    currentSliceLabel->setBuddy( currentSliceDisplay );
    currentSliceDisplay->setAlignment( Qt::AlignCenter );
    {
        auto pal = currentSliceDisplay->palette( );
        pal.setColor( QPalette::Background, Qt::black );
        currentSliceDisplay->setPalette( pal );
    }

    currentSliceLayout->setContentsMargins( { } );
    currentSliceLayout->addWidget( sliceProgressContainer );
    currentSliceLayout->addWidget( currentSliceLabel );
    currentSliceLayout->addWidget( currentSliceDisplay );
    currentSliceLayout->addStretch( );

    currentSliceContainer->setContentsMargins( { } );
    currentSliceContainer->setLayout( currentSliceLayout );
    currentSliceContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentSliceContainer->setMinimumSize( 600, 400 );

    _layout = new QGridLayout;
    _layout->setContentsMargins( { } );
    _layout->addWidget( optionsContainer,      0, 0, 1, 1 );
    _layout->addWidget( sliceButton,           1, 0, 1, 1 );
    _layout->addWidget( currentSliceContainer, 0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setContentsMargins( { } );
    setLayout( _layout );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

PrepareTab::~PrepareTab( ) {
    /*empty*/
}


void PrepareTab::layerThicknessComboBox_currentIndexChanged( int index ) {
    fprintf( stderr, "+ PrepareTab::layerThicknessComboBox_currentIndexChanged: new value: %d µm\n", LayerThicknessValues[index] );
    _printJob->layerThickness = LayerThicknessValues[index];
}

void PrepareTab::sliceButton_clicked( bool /*checked*/ ) {
    fprintf( stderr, "+ PrepareTab::sliceButton_clicked\n" );

    _printJob->pngFilesPath = StlModelLibraryPath + QString( "/working_%1" ).arg( static_cast<unsigned long long>( getpid( ) ) * 10000000000ull + static_cast<unsigned long long>( rand( ) ) );
    mkdir( _printJob->pngFilesPath.toUtf8( ).data( ), 0700 );

    QString baseName = _printJob->modelFileName;
    int index = baseName.lastIndexOf( "/" );
    if ( index > -1 ) {
        baseName = baseName.mid( index + 1 );
    }
    if ( baseName.endsWith( ".stl", Qt::CaseInsensitive ) ) {
        _printJob->slicedSvgFileName = _printJob->pngFilesPath + QChar( '/' ) + baseName.left( baseName.length( ) - 4 ) + QString( ".svg" );
    } else {
        _printJob->slicedSvgFileName = _printJob->pngFilesPath + QChar( '/' ) + baseName                                + QString( ".svg" );
    }

    fprintf( stderr,
        "  + model filename:      '%s'\n"
        "  + sliced SVG filename: '%s'\n"
        "  + PNG files path:      '%s'\n"
        "",
        _printJob->modelFileName.toUtf8( ).data( ),
        _printJob->slicedSvgFileName.toUtf8( ).data( ),
        _printJob->pngFilesPath.toUtf8( ).data( )
    );

    slicerProcess = new QProcess( this );
    slicerProcess->setProgram( SlicerCommand );
    slicerProcess->setArguments( QStringList {
        _printJob->modelFileName,
        "--export-svg",
        "--layer-height",
        QString( "%1" ).arg( _printJob->layerThickness / 1000.0 ),
        "--output",
        _printJob->slicedSvgFileName
    } );
    fprintf( stderr, "  + command line:        '%s %s'\n", slicerProcess->program( ).toUtf8( ).data( ), slicerProcess->arguments( ).join( QChar( ' ' ) ).toUtf8( ).data( ) );
    QObject::connect( slicerProcess, &QProcess::errorOccurred, this, &PrepareTab::slicerProcessErrorOccurred );
    QObject::connect( slicerProcess, &QProcess::started,       this, &PrepareTab::slicerProcessStarted       );
    QObject::connect( slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrepareTab::slicerProcessFinished );
    slicerProcess->start( );
}

void PrepareTab::setPrintJob( PrintJob* printJob ) {
    _printJob = printJob;
}

void PrepareTab::slicerProcessErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "+ PrepareTab::slicerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + slicer process failed to start\n" );
        sliceProgress->setText( "Slicer failed to start" );
    } else if ( QProcess::Crashed == error ) {
        fprintf( stderr, "  + slicer process crashed? state is %s [%d]\n", ToString( slicerProcess->state( ) ), slicerProcess->state( ) );
        if ( slicerProcess->state( ) != QProcess::NotRunning ) {
            slicerProcess->kill( );
            fprintf( stderr, "  + slicer terminated\n" );
        }
        sliceProgress->setText( "Slicer crashed" );
    }
}

void PrepareTab::slicerProcessStarted( ) {
    fprintf( stderr, "+ PrepareTab::slicerProcessStarted\n" );
    sliceProgress->setText( "Slicer started" );
    emit sliceStarting( );
}

void PrepareTab::slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    QObject::disconnect( slicerProcess, &QProcess::errorOccurred, this, &PrepareTab::slicerProcessErrorOccurred );
    QObject::disconnect( slicerProcess, &QProcess::started,       this, &PrepareTab::slicerProcessStarted       );
    QObject::disconnect( slicerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrepareTab::slicerProcessFinished );

    fprintf( stderr, "+ PrepareTab::slicerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    delete slicerProcess;
    slicerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        fprintf( stderr, "  + slicer process crashed?\n" );
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
            currentSliceDisplay->setPixmap( pixMap );
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

    QObject::disconnect( svgRenderer, &SvgRenderer::nextLayer, this, &PrepareTab::svgRenderer_progress );
    QObject::disconnect( svgRenderer, &SvgRenderer::done,      this, &PrepareTab::svgRenderer_done     );
    delete svgRenderer;
    svgRenderer = nullptr;

    emit renderComplete( totalLayers != -1 );
}
