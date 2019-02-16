#include "pch.h"

#include "calibrationtab.h"

CalibrationTab::CalibrationTab( QWidget* parent ): QWidget( parent ) {
    debug( "+ CalibrationTab::`ctor: constructing instance at %p\n", this );

    auto makeLabel = [] ( QString const& labelText, QSizePolicy::Policy const horizontalPolicy, QSizePolicy::Policy const verticalPolicy, QMargins const& margins = { } ) -> QLabel* {
        auto label = new QLabel( labelText );
        label->setContentsMargins( margins );
        label->setSizePolicy( horizontalPolicy, verticalPolicy );
        return label;
    };

    //
    // Left column
    //

    _stepsStatus.emplace_back( makeLabel( QString( "Step 1: Home printer"           ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    _stepsStatus.emplace_back( makeLabel( QString( "Step 2: Adjust build platform"  ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    _stepsStatus.emplace_back( makeLabel( QString( "Step 3: Load print solution"    ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    _stepsStatus.emplace_back( makeLabel( QString( "Step 4: Move to ready position" ), QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

    //_leftColumnLayout->setContentsMargins( { } );
    for ( auto widget : _stepsStatus ) {
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

    _rightColumn->setTitle( QString( "Calibration steps" ) );
    _rightColumn->setContentsMargins( { } );
    _rightColumn->setMinimumSize( MaximalRightHandPaneSize );
    _rightColumn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

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

    setContentsMargins( { } );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    setLayout( _layout );
}

CalibrationTab::~CalibrationTab( ) {
    debug( "+ CalibrationTab::`dtor: destroying instance at %p\n", this );
}

void CalibrationTab::_calibrateButton_clicked( bool ) {
    debug( "+ CalibrationTab::_calibrateButton_clicked\n" );
}
