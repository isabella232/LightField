#include "pch.h"

#include "statustab.h"
#include "printjob.h"
#include "strings.h"

StatusTab::StatusTab( QWidget* parent ): QWidget( parent ) {
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

    currentLayerImageLabel->setText( "Current layer:" );
    currentLayerImageLabel->setBuddy( currentLayerImageDisplay );

    currentLayerImageDisplay->setAlignment( Qt::AlignCenter );
    currentLayerImageDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    {
        auto pal = currentLayerImageDisplay->palette( );
        pal.setColor( QPalette::Background, Qt::black );
        currentLayerImageDisplay->setPalette( pal );
    }

    currentLayerImageLayout->setContentsMargins( { } );
    currentLayerImageLayout->addWidget( currentLayerImageLabel );
    currentLayerImageLayout->addWidget( currentLayerImageDisplay );
    currentLayerImageLayout->addStretch( );

    currentLayerImageContainer->setContentsMargins( { } );
    currentLayerImageContainer->setLayout( currentLayerImageLayout );
    currentLayerImageContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentLayerImageContainer->setMinimumSize( MaximalRightHandPaneSize );

    {
        auto font { stopButton->font( ) };
        font.setPointSizeF( 22.25 );
        stopButton->setFont( font );
    }
    stopButton->setText( "STOP" );
    stopButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    stopButton->setEnabled( false );
    QObject::connect( stopButton, &QPushButton::clicked, this, &StatusTab::stopButton_clicked );

    _layout->setContentsMargins( { } );
    _layout->addWidget( progressControlsContainer,  0, 0, 1, 1 );
    _layout->addWidget( stopButton,                 1, 0, 1, 1 );
    _layout->addWidget( currentLayerImageContainer, 0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

StatusTab::~StatusTab( ) {
    /*empty*/
}

void StatusTab::printer_Online( ) {
    debug( "+ StatusTab::printer_Online\n" );
    _isPrinterOnline = true;
    printerStateDisplay->setText( "Online" );
}

void StatusTab::printer_Offline( ) {
    debug( "+ StatusTab::printer_Offline\n" );
    _isPrinterOnline = false;
    printerStateDisplay->setText( "Offline" );
}

void StatusTab::stopButton_clicked( bool /*checked*/ ) {
    emit stopButtonClicked( );
}

void StatusTab::printManager_printStarting( ) {
    jobStateDisplay->setText( "Print started" );
}

void StatusTab::printManager_startingLayer( int const layer ) {
    currentLayerDisplay->setText( QString( "%1/%2" ).arg( layer + 1 ).arg( _printJob->layerCount ) );
    currentLayerImageDisplay->setPixmap( QPixmap( QString( "%1/%2.png" ).arg( _printJob->pngFilesPath ).arg( layer, 6, 10, QChar( '0' ) ) ) );
}

void StatusTab::printManager_lampStatusChange( bool const on ) {
    projectorLampStateDisplay->setText( QString( on ? "ON" : "off" ) );
}

void StatusTab::printManager_printComplete( bool const success ) {
    jobStateDisplay->setText( QString( success ? "Print complete" : "Print failed" ) );
}

void StatusTab::setPrintJob( PrintJob* printJob ) {
    _printJob = printJob;
}
