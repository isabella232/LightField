#include "pch.h"

#include "statustab.h"

#include "printjob.h"

StatusTab::StatusTab( QWidget* parent ): QWidget( parent ) {
    _initialShowEventFunc = std::bind( &StatusTab::_initialShowEvent, this );

    printerStateLabel->setText( "Printer status:" );
    printerStateLabel->setBuddy( printerStateDisplay );

    printerStateDisplay->setText( "Offline" );
    printerStateDisplay->setFrameShadow( QFrame::Sunken );
    printerStateDisplay->setFrameStyle( QFrame::StyledPanel );

    projectorLampStateLabel->setText( "Projector lamp status:" );
    projectorLampStateLabel->setBuddy( projectorLampStateDisplay );

    projectorLampStateDisplay->setText( "off" );
    projectorLampStateDisplay->setFrameShadow( QFrame::Sunken );
    projectorLampStateDisplay->setFrameStyle( QFrame::StyledPanel );

    jobStateLabel->setText( "Job status:" );
    jobStateLabel->setBuddy( jobStateDisplay );

    jobStateDisplay->setText( "Not printing" );
    jobStateDisplay->setFrameShadow( QFrame::Sunken );
    jobStateDisplay->setFrameStyle( QFrame::StyledPanel );

    currentLayerLabel->setText( "Current layer:" );
    currentLayerLabel->setBuddy( currentLayerDisplay );

    currentLayerDisplay->setFrameShadow( QFrame::Sunken );
    currentLayerDisplay->setFrameStyle( QFrame::StyledPanel );

    progressControlsLayout->setContentsMargins( { } );
    progressControlsLayout->addWidget( printerStateLabel );
    progressControlsLayout->addWidget( printerStateDisplay );
    progressControlsLayout->addWidget( projectorLampStateLabel );
    progressControlsLayout->addWidget( projectorLampStateDisplay );
    progressControlsLayout->addWidget( jobStateLabel );
    progressControlsLayout->addWidget( jobStateDisplay );
    progressControlsLayout->addWidget( currentLayerLabel );
    progressControlsLayout->addWidget( currentLayerDisplay );
    progressControlsLayout->addStretch( );

    progressControlsContainer->setContentsMargins( { } );
    progressControlsContainer->setLayout( progressControlsLayout );
    progressControlsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    currentLayerImage->setAlignment( Qt::AlignCenter );
    currentLayerImage->setContentsMargins( { } );
    {
        auto pal = currentLayerImage->palette( );
        pal.setColor( QPalette::Background, Qt::black );
        currentLayerImage->setPalette( pal );
    }

    currentLayerImageGroup->setTitle( "Current layer" );
    currentLayerImageGroup->setLayout( currentLayerImageLayout );
    currentLayerImageGroup->setContentsMargins( { } );
    currentLayerImageGroup->setMinimumSize( MaximalRightHandPaneSize );

    currentLayerImageLayout->setAlignment( Qt::AlignCenter );
    currentLayerImageLayout->setContentsMargins( { } );
    currentLayerImageLayout->addWidget( currentLayerImage );

    {
        auto font { stopButton->font( ) };
        font.setPointSizeF( 22.25 );
        font.setWeight( QFont::Bold );
        stopButton->setFont( font );

        _stopButtonEnabledPalette  = stopButton->palette( );
        _stopButtonDisabledPalette = _stopButtonEnabledPalette;
        _stopButtonEnabledPalette.setColor( QPalette::Button,     Qt::red    );
        _stopButtonEnabledPalette.setColor( QPalette::ButtonText, Qt::yellow );
    }
    stopButton->setText( "STOP" );
    stopButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    stopButton->setEnabled( false );
    QObject::connect( stopButton, &QPushButton::clicked, this, &StatusTab::stopButton_clicked );

    _layout->setContentsMargins( { } );
    _layout->addWidget( progressControlsContainer,  0, 0, 1, 1 );
    _layout->addWidget( stopButton,                 1, 0, 1, 1 );
    _layout->addWidget( currentLayerImageGroup,     0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

StatusTab::~StatusTab( ) {
    /*empty*/
}

void StatusTab::showEvent( QShowEvent* event ) {
    if ( _initialShowEventFunc ) {
        _initialShowEventFunc( );
        _initialShowEventFunc = nullptr;
    }
    event->ignore( );
}

void StatusTab::_initialShowEvent( ) {
    currentLayerImageGroup->setFixedSize( currentLayerImageGroup->size( ) );
    currentLayerImageGroup->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _maxLayerImageWidth = std::min( currentLayerImage->width( ), currentLayerImage->height( ) );
}

void StatusTab::printer_online( ) {
    debug( "+ StatusTab::printer_online\n" );
    _isPrinterOnline = true;
    printerStateDisplay->setText( "Online" );
}

void StatusTab::printer_offline( ) {
    debug( "+ StatusTab::printer_offline\n" );
    _isPrinterOnline = false;
    printerStateDisplay->setText( "Offline" );
}

void StatusTab::stopButton_clicked( bool ) {
    debug( "+ StatusTab::stopButton_clicked\n" );
    emit stopButtonClicked( );
}

void StatusTab::printManager_printStarting( ) {
    debug( "+ StatusTab::printManager_printStarting\n" );
    jobStateDisplay->setText( "Print started" );
}

void StatusTab::printManager_startingLayer( int const layer ) {
    debug( "+ StatusTab::printManager_startingLayer: layer %d/%d\n", layer, _printJob->layerCount );
    currentLayerDisplay->setText( QString( "%1/%2" ).arg( layer + 1 ).arg( _printJob->layerCount ) );
    auto pixmap = QPixmap( QString( "%1/%2.png" ).arg( _printJob->pngFilesPath ).arg( layer, 6, 10, QChar( '0' ) ) );
    if ( ( pixmap.width( ) > currentLayerImageGroup->width( ) ) || ( pixmap.height( ) > currentLayerImageGroup->height( ) ) ) {
        pixmap = pixmap.scaled( currentLayerImageGroup->width( ), currentLayerImageGroup->height( ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }
    currentLayerImage->setPixmap( pixmap );
}

void StatusTab::printManager_lampStatusChange( bool const on ) {
    debug( "+ StatusTab::printManager_lampStatusChange: lamp %s\n", on ? "ON" : "off" );
    projectorLampStateDisplay->setText( QString( on ? "ON" : "off" ) );
}

void StatusTab::printManager_printComplete( bool const success ) {
    debug( "+ StatusTab::printManager_printComplete: %s\n", success ? "Print complete" : "Print failed" );
    jobStateDisplay->setText( QString( success ? "Print complete" : "Print failed" ) );
    emit printComplete( );
}

void StatusTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ StatusTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void StatusTab::setStopButtonEnabled( bool value ) {
    stopButton->setEnabled( value );
    stopButton->setPalette( value ? _stopButtonEnabledPalette : _stopButtonDisabledPalette );
}
