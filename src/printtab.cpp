#include "pch.h"

#include "printtab.h"

#include "printjob.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"

using namespace std::placeholders;

namespace {

    char const* BuildPlatformStateStrings[] { "Lowered", "Raising", "Raised", "Lowering" };

    char const* ToString( BuildPlatformState const value ) {
#if defined _DEBUG
        if ( ( value >= BuildPlatformState::Lowered ) && ( value <= BuildPlatformState::Lowering ) ) {
#endif
            return BuildPlatformStateStrings[static_cast<int>( value )];
#if defined _DEBUG
        } else {
            return nullptr;
        }
#endif
    }

}

PrintTab::PrintTab( QWidget* parent ): QWidget( parent ) {
    _initialShowEventFunc = std::bind( &PrintTab::_initialShowEvent, this, _1 );

    auto origFont = font( );
    auto boldFont = ModifyFont( origFont, origFont.pointSizeF( ), QFont::Bold );


    exposureTimeLabel->setAlignment( Qt::AlignTop | Qt::AlignLeft );
    exposureTimeLabel->setText( "Exposure time (seconds):" );
    exposureTimeLabel->setBuddy( exposureTimeSlider );

    exposureTimeValue->setAlignment( Qt::AlignTop | Qt::AlignRight );
    exposureTimeValue->setFont( boldFont );

    exposureTimeValueLayout = WrapWidgetsInHBox( { exposureTimeLabel, nullptr, exposureTimeValue } );
    exposureTimeValueLayout->setContentsMargins( { } );

    exposureTimeSlider->setMinimum( 1 );
    exposureTimeSlider->setMaximum( 40 );
    exposureTimeSlider->setOrientation( Qt::Horizontal );
    exposureTimeSlider->setTickInterval( 2 );
    exposureTimeSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( exposureTimeSlider, &QDial::valueChanged, this, &PrintTab::exposureTimeSlider_valueChanged );


    exposureTimeLayout->setContentsMargins( { } );
    exposureTimeLayout->addLayout( exposureTimeValueLayout );
    exposureTimeLayout->addWidget( exposureTimeSlider );


    exposureTimeScaleFactorLabel->setAlignment( Qt::AlignTop | Qt::AlignLeft );
    exposureTimeScaleFactorLabel->setText( "First layers time scale factor:" );
    exposureTimeScaleFactorLabel->setBuddy( exposureTimeScaleFactorSlider );

    exposureTimeScaleFactorValue->setAlignment( Qt::AlignTop | Qt::AlignRight );
    exposureTimeScaleFactorValue->setFont( boldFont );

    exposureTimeScaleFactorValueLayout = WrapWidgetsInHBox( { exposureTimeScaleFactorLabel, nullptr, exposureTimeScaleFactorValue } );
    exposureTimeScaleFactorValueLayout->setContentsMargins( { } );

    exposureTimeScaleFactorSlider->setMinimum( 1 );
    exposureTimeScaleFactorSlider->setMaximum( 5 );
    exposureTimeScaleFactorSlider->setOrientation( Qt::Horizontal );
    exposureTimeScaleFactorSlider->setTickInterval( 1 );
    exposureTimeScaleFactorSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( exposureTimeScaleFactorSlider, &QSlider::valueChanged, this, &PrintTab::exposureTimeScaleFactorSlider_valueChanged );


    exposureTimeScaleFactorLayout->setContentsMargins( { } );
    exposureTimeScaleFactorLayout->addLayout( exposureTimeScaleFactorValueLayout );
    exposureTimeScaleFactorLayout->addWidget( exposureTimeScaleFactorSlider );


    exposureLayout->addLayout( exposureTimeLayout,            0, 0 );
    exposureLayout->addLayout( exposureTimeScaleFactorLayout, 0, 2 );
    exposureLayout->setRowStretch( 0, 1 );
    exposureLayout->setRowStretch( 1, 1 );
    exposureLayout->setColumnStretch( 0, 8 );
    exposureLayout->setColumnStretch( 1, 1 );
    exposureLayout->setColumnStretch( 2, 4 );


    powerLevelLabel->setText( "Projector power level:" );
    powerLevelLabel->setBuddy( powerLevelSlider );

    powerLevelValue->setAlignment( Qt::AlignRight );
    powerLevelValue->setFont( boldFont );

    powerLevelValueLayout = WrapWidgetsInHBox( { powerLevelLabel, nullptr, powerLevelValue } );
    powerLevelValueLayout->setContentsMargins( { } );

    powerLevelSlider->setMinimum( 20 );
    powerLevelSlider->setMaximum( 100 );
    powerLevelSlider->setOrientation( Qt::Horizontal );
    powerLevelSlider->setTickInterval( 1 );
    powerLevelSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( powerLevelSlider, &QDial::valueChanged, this, &PrintTab::powerLevelSlider_valueChanged );


    optionsLayout->setContentsMargins( { } );
    optionsLayout->addLayout( exposureLayout );
    optionsLayout->addLayout( powerLevelValueLayout );
    optionsLayout->addWidget( powerLevelSlider );
    optionsLayout->addStretch( );

    optionsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    optionsContainer->setLayout( optionsLayout );

    printButton->setEnabled( false );
    printButton->setFixedSize( MainButtonSize );
    printButton->setFont( ModifyFont( printButton->font( ), 22.0f ) );
    printButton->setText( "Print" );
    QObject::connect( printButton, &QPushButton::clicked, this, &PrintTab::printButton_clicked );

    _raiseOrLowerButton->setText( "Raise Build Platform" );
    _raiseOrLowerButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _raiseOrLowerButton, &QPushButton::clicked, this, &PrintTab::_raiseOrLowerButton_clicked );

    _homeButton->setText( "Home" );
    _homeButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _homeButton, &QPushButton::clicked, this, &PrintTab::_homeButton_clicked );

    _adjustmentsGroup->setTitle( QString( "Adjustments" ) );
    _adjustmentsGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _adjustmentsGroup->setFixedHeight( MainButtonSize.height( ) );
    _adjustmentsGroup->setLayout( WrapWidgetsInHBox( { nullptr, _homeButton, nullptr, _raiseOrLowerButton, nullptr } ) );

    _layout->setContentsMargins( { } );
    _layout->addWidget( optionsContainer,  0, 0, 1, 2 );
    _layout->addWidget( printButton,       1, 0, 1, 1 );
    _layout->addWidget( _adjustmentsGroup, 1, 1, 1, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

PrintTab::~PrintTab( ) {
    /*empty*/
}

void PrintTab::showEvent( QShowEvent* event ) {
    if ( _initialShowEventFunc ) {
        _initialShowEventFunc( event );
        _initialShowEventFunc = nullptr;
    } else {
        event->ignore( );
    }
}

void PrintTab::_initialShowEvent( QShowEvent* event ) {
    auto size = QSize {
        std::max( { _raiseOrLowerButton->width( ),  _homeButton->width( ),  } ),
        std::max( { _raiseOrLowerButton->height( ), _homeButton->height( ), } )
    };

    _raiseOrLowerButton->setFixedSize( size );
    _homeButton        ->setFixedSize( size );

    event->accept( );
}

void PrintTab::exposureTimeSlider_valueChanged( int value ) {
    _printJob->exposureTime = value / 2.0;
    exposureTimeValue->setText( QString( "%1 s" ).arg( _printJob->exposureTime, 0, 'f', 1 ) );
}

void PrintTab::exposureTimeScaleFactorSlider_valueChanged( int value ) {
    _printJob->exposureTimeScaleFactor = value;
    exposureTimeScaleFactorValue->setText( QString( "%1×" ).arg( value ) );
}

void PrintTab::powerLevelSlider_valueChanged( int value ) {
    _printJob->powerLevel = value / 100.0 * 255.0 + 0.5;
    powerLevelValue->setText( QString( "%1%" ).arg( value ) );
}

void PrintTab::printButton_clicked( bool ) {
    debug( "+ PrintTab::printButton_clicked\n" );
    emit printButtonClicked( );
}

void PrintTab::_raiseOrLowerButton_clicked( bool ) {
    debug( "+ PrintTab::_raiseOrLowerButton_clicked: _buildPlatformState %s [%d]\n", ToString( _buildPlatformState ), _buildPlatformState );
    setAdjustmentButtonsEnabled( false );

    switch ( _buildPlatformState ) {
        case BuildPlatformState::Lowered:
        case BuildPlatformState::Raising:
            _buildPlatformState = BuildPlatformState::Raising;
            emit raiseBuildPlatform( );
            break;

        case BuildPlatformState::Raised:
        case BuildPlatformState::Lowering:
            _buildPlatformState = BuildPlatformState::Lowering;
            emit lowerBuildPlatform( );
            break;

        default:
            return;
    }
}

void PrintTab::_homeButton_clicked( bool ) {
    debug( "+ PrintTab::_homeButton_clicked\n" );
    setAdjustmentButtonsEnabled( false );

    emit homePrinter( );
}

void PrintTab::raiseBuildPlatformComplete( bool const success ) {
    debug( "+ PrintTab::raiseBuildPlatformComplete: %s\n", success ? "succeeded" : "failed" );
    if ( success ) {
        _buildPlatformState = BuildPlatformState::Raised;
        _raiseOrLowerButton->setText( "Lower Build Platform" );
        _raiseOrLowerButton->setEnabled( true );
    } else {
        _buildPlatformState = BuildPlatformState::Lowered;
        setAdjustmentButtonsEnabled( true );
    }
}

void PrintTab::homeComplete( bool const success ) {
    debug( "+ PrintTab::homeComplete: %s\n", success ? "succeeded" : "failed" );
    setAdjustmentButtonsEnabled( true );
}

void PrintTab::lowerBuildPlatformComplete( bool const success ) {
    debug( "+ PrintTab::lowerBuildPlatformComplete: %s\n", success ? "succeeded" : "failed" );
    if ( success ) {
        _buildPlatformState = BuildPlatformState::Lowered;
        _raiseOrLowerButton->setText( "Raise Build Platform" );
    } else {
        _buildPlatformState = BuildPlatformState::Raised;
    }
    setAdjustmentButtonsEnabled( true );
}

void PrintTab::setAdjustmentButtonsEnabled( bool const value ) {
    debug( "+ PrintTab::setAdjustmentButtonsEnabled: value %s\n", value ? "enabled" : "disabled" );
    _raiseOrLowerButton->setEnabled( value );
    _homeButton        ->setEnabled( value );
}

void PrintTab::setPrintButtonEnabled( bool const value ) {
    debug( "+ PrintTab::setPrintButtonEnabled: value %s\n", value ? "enabled" : "disabled" );
    printButton->setEnabled( value );
}

void PrintTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ PrintTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;

    int value = _printJob->exposureTime / 0.5;
    _printJob->exposureTime = value / 2.0;
    exposureTimeSlider->setValue( value );
    exposureTimeValue->setText( QString( "%1 s" ).arg( _printJob->exposureTime, 0, 'f', 1 ) );

    debug( "+ PrintTab::setPrintJob: _printJob->exposureTimeScaleFactor=%f\n", _printJob->exposureTimeScaleFactor );
    exposureTimeScaleFactorSlider->setValue( _printJob->exposureTimeScaleFactor );
    exposureTimeScaleFactorValue->setText( QString( "%1×" ).arg( _printJob->exposureTimeScaleFactor ) );

    powerLevelSlider->setValue( _printJob->powerLevel / 255.0 * 100.0 + 0.5 );
    powerLevelValue->setText( QString( "%1%" ).arg( static_cast<int>( _printJob->powerLevel / 255.0 * 100.0 + 0.5 ) ) );
}

void PrintTab::setShepherd( Shepherd* newShepherd ) {
    _shepherd = newShepherd;
}
