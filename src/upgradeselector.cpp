#include "pch.h"

#include "upgradeselector.h"

#include "app.h"
#include "strings.h"
#include "upgrademanager.h"
#include "utils.h"

UpgradeSelector::UpgradeSelector( UpgradeManager* upgradeManager, QWidget* parent ): InitialShowEventMixin<UpgradeSelector, QMainWindow>( parent ), _upgradeManager( upgradeManager ) {
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    setFixedSize( MainWindowSize );
    move( g_settings.mainWindowPosition );

    setFont( ModifyFont( font( ), 22.0 ) );

    auto availableKitsLabel = new QLabel { "Available versions:" };

    QStringList kitsListStrings;
    for ( auto const& kitInfo : _upgradeManager->availableUpgrades( ) ) {
        if ( kitInfo.buildType == BuildType::Debug ) {
            kitsListStrings.append( kitInfo.versionString % " (debug version)" );
        } else {
            kitsListStrings.append( kitInfo.versionString );
        }
    }

    auto kitsListView = new GestureListView;
    kitsListView->setFlow( QListView::TopToBottom );
    kitsListView->setLayoutMode( QListView::SinglePass );
    kitsListView->setMovement( QListView::Static );
    kitsListView->setSelectionMode( QListView::SingleSelection );
    kitsListView->setViewMode( QListView::ListMode );
    kitsListView->setModel( new QStringListModel { kitsListStrings } );
    QObject::connect( kitsListView, &GestureListView::clicked, this, &UpgradeSelector::kitsListView_clicked );

    _upgradeButton = new QPushButton { "Upgrade" };
    _upgradeButton->setDefault( false );
    _upgradeButton->setEnabled( false );
    QObject::connect( _upgradeButton, &QPushButton::clicked, this, &UpgradeSelector::upgradeButton_clicked );

    _cancelButton = new QPushButton { "Cancel" };
    _cancelButton->setDefault( true );
    QObject::connect( _cancelButton,  &QPushButton::clicked, this, &UpgradeSelector::cancelButton_clicked );

    {
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

    {
        _upgradeInProgressMessage = new QLabel { "Please wait, software update in progress..." };

        auto verticalLayout = WrapWidgetsInVBox( { nullptr, _upgradeInProgressMessage, nullptr } );

        _upgradeInProgressLayout = new QHBoxLayout;
        _upgradeInProgressLayout->addStretch( );
        _upgradeInProgressLayout->addLayout( verticalLayout );
        _upgradeInProgressLayout->addStretch( );
    }

    {
        _upgradeFailedMessage = new QLabel { "Software update failed." };

        _okButton = new QPushButton { "OK" };
        QObject::connect( _okButton, &QPushButton::clicked, this, &UpgradeSelector::okButton_clicked );

        auto verticalLayout = WrapWidgetsInVBox( { nullptr, _upgradeFailedMessage, nullptr, _okButton, nullptr } );

        _upgradeFailedLayout = new QHBoxLayout;
        _upgradeFailedLayout->addStretch( );
        _upgradeFailedLayout->addLayout( verticalLayout );
        _upgradeFailedLayout->addStretch( );
    }
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
    debug( "+ UpgradeSelector::upgradeButton_clicked\n" );
    emit kitSelected( _upgradeManager->availableUpgrades( )[_currentSelection] );
}

void UpgradeSelector::cancelButton_clicked( bool ) {
    debug( "+ UpgradeSelector::cancelButton_clicked\n" );
    emit canceled( );
}

void UpgradeSelector::okButton_clicked( bool ) {
    debug( "+ UpgradeSelector::okButton_clicked\n" );
    emit canceled( );
}

void UpgradeSelector::showInProgressMessage( ) {
    debug( "+ UpgradeSelector::showInProgressMessage\n" );

    auto centralWidget = new QWidget;
    centralWidget->setLayout( _upgradeInProgressLayout );
    setCentralWidget( centralWidget );
}

void UpgradeSelector::showFailedMessage( ) {
    debug( "+ UpgradeSelector::showFailedMessage\n" );

    auto centralWidget = new QWidget;
    centralWidget->setLayout( _upgradeFailedLayout );
    setCentralWidget( centralWidget );
}
