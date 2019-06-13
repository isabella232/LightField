#include "pch.h"

#include "upgradeselector.h"

#include "app.h"
#include "strings.h"
#include "upgrademanager.h"
#include "utils.h"

UpgradeSelector::UpgradeSelector( UpgradeManager* upgradeManager, QWidget* parent ): InitialShowEventMixin<UpgradeSelector, QMainWindow>( parent ), _upgradeManager( upgradeManager ) {
    setFont( ModifyFont( font( ), 22.0 ) );

    auto availableKitsLabel = new QLabel { "Available versions:" };

    QStringList kitsListStrings;
    for ( auto const& kitInfo : _upgradeManager->availableUpgrades( ) ) {
        if ( kitInfo.buildType == BuildType::Debug ) {
            kitsListStrings.insert( kitsListStrings.end( ), kitInfo.versionString % " (debug version)" );
        } else {
            kitsListStrings.insert( kitsListStrings.end( ), kitInfo.versionString );
        }
    }

    auto kitsListModel = new QStringListModel { kitsListStrings };

    auto kitsListView = new GestureListView;
    kitsListView->setFlow( QListView::TopToBottom );
    kitsListView->setLayoutMode( QListView::SinglePass );
    kitsListView->setMovement( QListView::Static );
    kitsListView->setSelectionMode( QListView::SingleSelection );
    kitsListView->setViewMode( QListView::ListMode );
    kitsListView->setModel( kitsListModel );
    QObject::connect( kitsListView, &GestureListView::clicked, this, &UpgradeSelector::kitsListView_clicked );

    _upgradeButton = new QPushButton { "Upgrade" };
    _upgradeButton->setDefault( false );
    _upgradeButton->setEnabled( false );
    QObject::connect( _upgradeButton, &QPushButton::clicked, this, &UpgradeSelector::upgradeButton_clicked );

    _cancelButton = new QPushButton { "Cancel" };
    _cancelButton->setDefault( true );
    QObject::connect( _cancelButton,  &QPushButton::clicked, this, &UpgradeSelector::cancelButton_clicked );

    auto verticalLayout = new QVBoxLayout;
    verticalLayout->addStretch( );
    verticalLayout->addWidget( availableKitsLabel );
    verticalLayout->addWidget( kitsListView );
    verticalLayout->addLayout( WrapWidgetsInHBox( { nullptr, _upgradeButton, nullptr, _cancelButton, nullptr } ) );
    verticalLayout->addStretch( );

    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch( );
    horizontalLayout->addLayout( verticalLayout );
    horizontalLayout->addStretch( );

    auto centralWidget = new QWidget;
    centralWidget->setLayout( horizontalLayout );
    setCentralWidget( centralWidget );
}

UpgradeSelector::~UpgradeSelector( ) {
    /*empty*/
}

void UpgradeSelector::_initialShowEvent( QShowEvent* event ) {
    move( g_settings.mainWindowPosition );
    setFixedSize( MainWindowSize );

    auto newSize = maxSize( _upgradeButton->size( ), _cancelButton->size( ) ) + QSize { 20, 4 };
    _upgradeButton->setFixedSize( newSize );
    _cancelButton->setFixedSize( newSize );

    event->accept( );
}

void UpgradeSelector::kitsListView_clicked( QModelIndex const& index ) {
    if ( index.isValid( ) ) {
        auto row = index.row( );
        auto kitInfo = _upgradeManager->availableUpgrades( )[row];
        debug( "+ UpgradeSelector::kitsListView_clicked: row %d selected\n", row );
        debug( "  + version:    %s\n", kitInfo.versionString.toUtf8( ).data( ) );
        debug( "  + build type: %s\n", ToString( kitInfo.buildType ) );
        _currentSelection = row;
        _upgradeButton->setEnabled( true );
    } else {
        debug( "+ UpgradeSelector::kitsListView_clicked: item deselected\n" );
        _currentSelection = -1;
        _upgradeButton->setEnabled( false );
    }
}

void UpgradeSelector::upgradeButton_clicked( bool ) {
    emit kitSelected( _upgradeManager->availableUpgrades( )[_currentSelection] );
}

void UpgradeSelector::cancelButton_clicked( bool ) {
    emit canceled( );
}
