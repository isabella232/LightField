#include "pch.h"

#include "calibrationtab.h"

#include "shepherd.h"

namespace {

    QLabel* makeLabel( QString const& labelText, QSizePolicy::Policy const horz, QSizePolicy::Policy const vert ) {
        auto label = new QLabel( labelText );
        label->setSizePolicy( horz, vert );
        return label;
    };

}

/*
    0. "Calibrate" button.

    1. G28 X -- home on X axis
    2. wait for i) to complete
    3. "Adjust the build platform position now."
    4. G92 X0, G0 X50 -- get out of the way for resin load
    5. wait for iv) to complete
    6. "Load print solution now."
    7. G90, G0 X0.1
    8. wait for vii) to complete

    9. enable Print button
*/

CalibrationTab::CalibrationTab( QWidget* parent ): QWidget( parent ) {
    debug( "+ CalibrationTab::`ctor: constructing instance at %p\n", this );

    //
    // Left column
    //

    _stepsStatus.emplace_back( makeLabel( QString( "Step 1: Move to home position"  ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    _stepsStatus.emplace_back( makeLabel( QString( "Step 2: Adjust build platform"  ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    _stepsStatus.emplace_back( makeLabel( QString( "Step 3: Load print solution"    ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    _stepsStatus.emplace_back( makeLabel( QString( "Step 4: Move to ready position" ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

    for ( auto widget : _stepsStatus ) {
        widget->setEnabled( false );
        _leftColumnLayout->addWidget( widget );
    }
    _leftColumnLayout->addStretch( );

    //
    // Bottom-left button
    //

    {
        auto font { _calibrateButton->font( ) };
        font.setPointSizeF( 22.25 );
        _calibrateButton->setFont( font );
    }
    _calibrateButton->setText( QString( "Calibrate" ) );
    _calibrateButton->setEnabled( true );
    _calibrateButton->setContentsMargins( { } );
    _calibrateButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    QObject::connect( _calibrateButton, &QPushButton::clicked, this, &CalibrationTab::_calibrateButton_clicked );

    //
    // Right column
    //

    {
        auto font { _calibrationMessage->font( ) };
        font.setPointSizeF( 16.25 );
        _calibrationMessage->setFont( font );
    }
    _calibrationMessage->setTextFormat( Qt::RichText );
    _calibrationMessage->setText( QString( "Tap the <b>Calibrate</b> button to begin." ) );

    _calibrationProgress->setRange( 0, 0 );
    _calibrationProgress->hide( );

    _continueButton->setText( QString( "Continue" ) );
    _continueButton->setEnabled( false );

    _calibrationStepsLayout->setContentsMargins( { } );
    _calibrationStepsLayout->addStretch( ); _calibrationStepsLayout->addWidget( _calibrationMessage,  0, Qt::AlignCenter );
    _calibrationStepsLayout->addStretch( ); _calibrationStepsLayout->addWidget( _calibrationProgress, 0, Qt::AlignCenter );
    _calibrationStepsLayout->addStretch( ); _calibrationStepsLayout->addWidget( _continueButton,      0, Qt::AlignCenter );
    _calibrationStepsLayout->addStretch( );

    _rightColumn->setTitle( QString( "Calibration steps" ) );
    _rightColumn->setContentsMargins( { } );
    _rightColumn->setMinimumSize( MaximalRightHandPaneSize );
    _rightColumn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _rightColumn->setLayout( _calibrationStepsLayout );

    _rightColumnLayout->addWidget( _rightColumn );

    //
    // Layouts, assemble!
    //

    _layout->setContentsMargins( { } );
    _layout->addLayout( _leftColumnLayout,  0, 0, 1, 1 );
    _layout->addWidget( _calibrateButton,   1, 0, 1, 1 );
    _layout->addLayout( _rightColumnLayout, 0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    setLayout( _layout );
}

CalibrationTab::~CalibrationTab( ) {
    debug( "+ CalibrationTab::`dtor: destroying instance at %p\n", this );
}

void CalibrationTab::_calibrateButton_clicked( bool ) {
    debug( "+ CalibrationTab::_calibrateButton_clicked\n" );

    _calibrateButton->setEnabled( false );
    _calibrationMessage->setText( QString( "Moving the printer to its home location..." ) );
    _calibrationProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &CalibrationTab::_sendHome_complete );
    _shepherd->doSend( "G28 X" );
}

void CalibrationTab::_sendHome_complete( bool const success ) {
    debug( "+ CalibrationTab::_sendHome_complete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );

    if ( !success ) {
        _calibrationMessage->setText( QString( "Calibration failed." ) );
        _calibrationProgress->hide( );

        emit calibrationComplete( false );
        return;
    }

    _calibrationMessage->setText( QString( "<div style='text-align: center;'>Adjust the build platform position,<br>then tap <b>Continue</b>.</div>" ) );
    _calibrationProgress->hide( );

    QObject::connect( _continueButton, &QPushButton::clicked, this, &CalibrationTab::_adjustBuildPlatform_complete );
    _continueButton->setEnabled( true );
}

void CalibrationTab::_adjustBuildPlatform_complete( bool ) {
    debug( "+ CalibrationTab::_adjustBuildPlatform_complete\n" );

    QObject::disconnect( _continueButton, nullptr, this, nullptr );
    _continueButton->setEnabled( false );

    _calibrationMessage->setText( QString( "<div style='text-align: center;'>Moving the build platform<br>out of the way...</div>" ) );
    _calibrationProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &CalibrationTab::_sendResinLoadMove_complete );
    _shepherd->doSend( QStringList { "G92 X0", "G0 X50" }  );
}

void CalibrationTab::_sendResinLoadMove_complete( bool const success ) {
    debug( "+ CalibrationTab::_sendResinLoadMove_complete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );

    if ( !success ) {
        _calibrationMessage->setText( QString( "Calibration failed." ) );
        _calibrationProgress->hide( );

        emit calibrationComplete( false );
        return;
    }

    _calibrationMessage->setText( QString( "<div style='text-align: center;'>Load the print solution,<br>then tap <b>Continue</b>.</div>" ) );
    _calibrationProgress->hide( );

    QObject::connect( _continueButton, &QPushButton::clicked, this, &CalibrationTab::_loadPrintSolution_complete );
    _continueButton->setEnabled( true );
}

void CalibrationTab::_loadPrintSolution_complete( bool ) {
    debug( "+ CalibrationTab::_loadPrintSolution_complete\n" );

    QObject::disconnect( _continueButton, nullptr, this, nullptr );
    _continueButton->setEnabled( false );

    _calibrationMessage->setText( QString( "Extending the build platform..." ) );
    _calibrationProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &CalibrationTab::_sendExtend_complete );
    _shepherd->doSend( QStringList { "G90", "G0 X0.1" }  );
}

void CalibrationTab::_sendExtend_complete( bool const success ) {
    debug( "+ CalibrationTab::_sendExtend_complete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );

    if ( !success ) {
        _calibrationMessage->setText( QString( "Calibration failed." ) );
        _calibrationProgress->hide( );

        emit calibrationComplete( false );
        return;
    }

    _calibrationMessage->setText( QString( "Calibration completed." ) );
    _calibrationProgress->hide( );

    emit calibrationComplete( true );
}

void CalibrationTab::setShepherd( Shepherd* shepherd ) {
    _shepherd = shepherd;
}
