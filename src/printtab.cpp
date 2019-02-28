#include "pch.h"

#include "printtab.h"

#include "printjob.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"

using namespace std::placeholders;

namespace {

    QStringList ExposureTimeScaleFactorStringList { "1×", "2×", "3×", "4×", "5×" };
    double      ExposureTimeScaleFactorValues[]   { 1.0,  2.0,  3.0,  4.0,  5.0  };

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

    exposureTimeDial->setMinimum(  1 );
    exposureTimeDial->setMaximum( 40 );
    exposureTimeDial->setNotchesVisible( true );
    exposureTimeDial->setWrapping( false );
    QObject::connect( exposureTimeDial, &QDial::valueChanged, this, &PrintTab::exposureTimeDial_valueChanged );

    exposureTimeLabel->setText( "Exposure time (seconds):" );
    exposureTimeLabel->setBuddy( exposureTimeDial );

    exposureTimeValue->setAlignment( Qt::AlignRight );
    exposureTimeValue->setFont( boldFont );

    exposureTimeValueLayout = WrapWidgetsInHBox( { exposureTimeLabel, nullptr, exposureTimeValue } );
    exposureTimeValueLayout->setContentsMargins( { } );

    exposureTimeValueContainer->setContentsMargins( { } );
    exposureTimeValueContainer->setLayout( exposureTimeValueLayout );

    exposureTimeDialLeftLabel->setAlignment( Qt::AlignLeft );
    exposureTimeDialLeftLabel->setContentsMargins( { } );
    exposureTimeDialLeftLabel->setText( "0.5 s" );

    exposureTimeDialRightLabel->setAlignment( Qt::AlignRight );
    exposureTimeDialRightLabel->setContentsMargins( { } );
    exposureTimeDialRightLabel->setText( "20 s" );

    exposureTimeDialLabelsLayout = WrapWidgetsInHBox( { nullptr, exposureTimeDialLeftLabel, nullptr, exposureTimeDialRightLabel, nullptr } );
    exposureTimeDialLabelsLayout->setContentsMargins( { } );

    exposureTimeDialLabelsContainer->setContentsMargins( { } );
    exposureTimeDialLabelsContainer->setLayout( exposureTimeDialLabelsLayout );

    exposureTimeScaleFactorComboBox->setEditable( false );
    exposureTimeScaleFactorComboBox->setMaxVisibleItems( ExposureTimeScaleFactorStringList.count( ) );
    exposureTimeScaleFactorComboBox->addItems( ExposureTimeScaleFactorStringList );
    QObject::connect( exposureTimeScaleFactorComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &PrintTab::exposureTimeScaleFactorComboBox_currentIndexChanged );

    exposureTimeScaleFactorLabel->setText( "First layers time scale factor:" );
    exposureTimeScaleFactorLabel->setBuddy( exposureTimeScaleFactorComboBox );

    powerLevelDial->setMinimum( 20 );
    powerLevelDial->setMaximum( 100 );
    powerLevelDial->setNotchesVisible( true );
    powerLevelDial->setWrapping( false );
    QObject::connect( powerLevelDial, &QDial::valueChanged, this, &PrintTab::powerLevelDial_valueChanged );

    powerLevelLabel->setText( "Projector power level:" );
    powerLevelLabel->setBuddy( powerLevelDial );

    powerLevelValue->setAlignment( Qt::AlignRight );
    powerLevelValue->setFont( boldFont );

    powerLevelValueLayout = WrapWidgetsInHBox( { powerLevelLabel, nullptr, powerLevelValue } );
    powerLevelValueLayout->setContentsMargins( { } );

    powerLevelValueContainer->setContentsMargins( { } );
    powerLevelValueContainer->setLayout( powerLevelValueLayout );

    powerLevelDialLeftLabel->setAlignment( Qt::AlignLeft );
    powerLevelDialLeftLabel->setContentsMargins( { } );
    powerLevelDialLeftLabel->setText( "20%" );
    powerLevelDialLeftLabel->setBuddy( powerLevelDial );

    powerLevelDialRightLabel->setAlignment( Qt::AlignRight );
    powerLevelDialRightLabel->setContentsMargins( { } );
    powerLevelDialRightLabel->setText( "100%" );
    powerLevelDialRightLabel->setBuddy( powerLevelDial );

    powerLevelDialLabelsLayout = WrapWidgetsInHBox( { nullptr, powerLevelDialLeftLabel, nullptr, powerLevelDialRightLabel, nullptr } );
    powerLevelDialLabelsLayout->setContentsMargins( { } );

    powerLevelDialLabelsContainer->setContentsMargins( { } );
    powerLevelDialLabelsContainer->setLayout( powerLevelDialLabelsLayout );

    optionsLayout = WrapWidgetsInVBox( {
        exposureTimeValueContainer, exposureTimeDial, exposureTimeDialLabelsContainer,
        exposureTimeScaleFactorLabel, exposureTimeScaleFactorComboBox,
        powerLevelValueContainer, powerLevelDial, powerLevelDialLabelsContainer,
        nullptr
    } );
    optionsLayout->setContentsMargins( { } );

    optionsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    optionsContainer->setLayout( optionsLayout );

    printButton->setEnabled( false );
    printButton->setFixedSize( MainButtonSize );
    printButton->setFont( ModifyFont( printButton->font( ), 22.0f ) );
    printButton->setText( "Print" );
    QObject::connect( printButton, &QPushButton::clicked, this, &PrintTab::printButton_clicked );

    _raiseOrLowerButton->setText( "Raise\nBuild Platform" );
    _raiseOrLowerButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _raiseOrLowerButton, &QPushButton::clicked, this, &PrintTab::_raiseOrLowerButton_clicked );

    _homeButton->setText( "Home" );
    _homeButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _homeButton, &QPushButton::clicked, this, &PrintTab::_homeButton_clicked );

    _raiseOrLowerLayout->addStretch( );
    _raiseOrLowerLayout->addWidget( _raiseOrLowerButton, 0, Qt::AlignCenter );
    _raiseOrLowerLayout->addStretch( );

    _homeLayout->addStretch( );
    _homeLayout->addWidget( _homeButton, 0, Qt::AlignCenter );
    _homeLayout->addStretch( );

    _raiseOrLowerGroup->setMinimumSize( QuarterRightHandPaneSize );
    _raiseOrLowerGroup->setLayout( _raiseOrLowerLayout );
    _raiseOrLowerGroup->setTitle( "Build Platform" );

    _homeGroup->setMinimumSize( QuarterRightHandPaneSize );
    _homeGroup->setLayout( _homeLayout );
    _homeGroup->setTitle( "Home" );

    _adjustmentsLayout->addWidget( _raiseOrLowerGroup, 1, 0, 1, 1, Qt::AlignCenter );
    _adjustmentsLayout->addWidget( _homeGroup,         1, 1, 1, 1, Qt::AlignCenter );
    _adjustmentsLayout->setRowStretch( 0, 1 );
    _adjustmentsLayout->setRowStretch( 1, 1 );
    _adjustmentsLayout->setColumnStretch( 0, 1 );
    _adjustmentsLayout->setColumnStretch( 1, 1 );

    _adjustmentsGroup->setTitle( QString( "Adjustments" ) );
    _adjustmentsGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _adjustmentsGroup->setMinimumSize( MaximalRightHandPaneSize );
    _adjustmentsGroup->setLayout( _adjustmentsLayout );

    _layout->setContentsMargins( { } );
    _layout->addWidget( optionsContainer,  0, 0, 1, 1 );
    _layout->addWidget( printButton,       1, 0, 1, 1 );
    _layout->addWidget( _adjustmentsGroup, 0, 1, 2, 1 );
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

void PrintTab::exposureTimeDial_valueChanged( int value ) {
    _printJob->exposureTime = value / 2.0;
    exposureTimeValue->setText( QString( "%1 s" ).arg( _printJob->exposureTime, 0, 'f', 1 ) );
}

void PrintTab::exposureTimeScaleFactorComboBox_currentIndexChanged( int index ) {
    _printJob->exposureTimeScaleFactor = ExposureTimeScaleFactorValues[index];
}

void PrintTab::powerLevelDial_valueChanged( int value ) {
    int scaledValue = ( value / 100.0 * 255.0 ) + 0.5;
    _printJob->powerLevel = scaledValue;
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
        _raiseOrLowerButton->setText( "Lower\nBuild Platform" );
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
        _raiseOrLowerButton->setText( "Raise\nBuild Platform" );
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
    exposureTimeDial->setValue( value );

    auto exposureTimeScaleFactorText = FormatDouble( _printJob->exposureTimeScaleFactor ) + QString( "×" );
    int index = exposureTimeScaleFactorComboBox->findText( exposureTimeScaleFactorText );
    if ( -1 == index ) {
        debug( "  + couldn't find exposureTimeScaleFactorComboBox entry for %f => '%s'\n", _printJob->exposureTimeScaleFactor, exposureTimeScaleFactorText.toUtf8( ).data( ) );
    } else {
        exposureTimeScaleFactorComboBox->setCurrentIndex( index );
    }

    powerLevelDial->setValue( _printJob->powerLevel / 255.0 * 100.0 + 0.5 );
}

void PrintTab::setShepherd( Shepherd* newShepherd ) {
    _shepherd = newShepherd;
}
