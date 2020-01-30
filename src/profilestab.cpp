#include "pch.h"

#include "profilestab.h"
#include "profilesjsonparser.h"
#include "window.h"

#include <dirent.h>
#include <iostream>

ProfilesTab::ProfilesTab( QWidget* parent ): TabBase( parent ) {
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome", LargeFontSize );
    _fontAwesome = new QFont(fontAwesome);


    QGroupBox* cpyProfilesUsbBox = new QGroupBox();
    cpyProfilesUsbBox->setLayout(WrapWidgetsInVBox(_cpyProfilesUsb, nullptr));
    cpyProfilesUsbBox->setContentsMargins( { } );

    QGroupBox* cpyStlFilesUsbBox = new QGroupBox();
    cpyStlFilesUsbBox->setLayout(WrapWidgetsInVBox(_cpyStlFilesUsb, nullptr));
    cpyStlFilesUsbBox->setContentsMargins( { } );

    _cpyProfilesUsb->setFont(fontAwesome);
    _cpyStlFilesUsb->setFont(fontAwesome);

    _importParams->setFont(fontAwesome);
    _importParams->setFixedSize( MainButtonSize );
    _importParams->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _exportParams->setFont(fontAwesome);
    _exportParams->setFixedSize( MainButtonSize );
    _exportParams->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _newProfile->setFont(fontAwesome);
    _newProfile->setFixedSize( MainButtonSize );
    _newProfile->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _deleteProfile->setFont(fontAwesome);
    _deleteProfile->setFixedSize( MainButtonSize );
    _deleteProfile->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _loadProfile->setFont(fontAwesome);
    _loadProfile->setFixedSize( MainButtonSize );
    _loadProfile->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _setupProfilesList(fontAwesome);

    QObject::connect( _importParams, &QPushButton::clicked, this, &ProfilesTab::importParams_clicked );
    QObject::connect( _exportParams, &QPushButton::clicked, this, &ProfilesTab::exportParams_clicked );
    QObject::connect( _newProfile, &QPushButton::clicked, this, &ProfilesTab::newProfile_clicked );
    QObject::connect( _deleteProfile, &QPushButton::clicked, this, &ProfilesTab::deleteProfile_clicked );
    QObject::connect( _loadProfile, &QPushButton::clicked, this, &ProfilesTab::loadProfile_clicked );

    setLayout(
        WrapWidgetsInHBox(
              WrapWidgetsInVBox(
                   _exportParams,
                   cpyProfilesUsbBox,
                   cpyStlFilesUsbBox,
                   _importParams,
                   _newProfile,
                   _loadProfile,
                   _deleteProfile,
                   nullptr
              ),
              _profilesList
        )
    );
}

ProfilesTab::~ProfilesTab( ) {
    /*empty*/
}

void ProfilesTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ ProfilesTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch ( _uiState ) {
        case UiState::SelectStarted:
        case UiState::SelectCompleted:
        case UiState::SliceStarted:
        case UiState::SliceCompleted:
        case UiState::PrintStarted:
        case UiState::PrintCompleted:
            break;
    }
}

void ProfilesTab::_setupProfilesList(QFont font) {

    QItemSelectionModel* selectionModel = new QItemSelectionModel( _model );
    _profilesList->setModel( _model );
    _profilesList->setSelectionModel( selectionModel );
    _profilesList->setFont( font );
    _profilesList->setVisible( true );
    _profilesList->setSelectionBehavior( QAbstractItemView::SelectRows );
    _profilesList->setMinimumSize(QSize(MaximalRightHandPaneSize.width(), MaximalRightHandPaneSize.height()));
    _profilesList->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

}

void ProfilesTab::setPrintProfileManager( PrintProfileManager* printProfileManager ) {
    _printProfileManager = printProfileManager;

    QStandardItem* item = nullptr;
    QStandardItem* firstItem = nullptr;
    QVector<PrintProfile*>* profiles = ProfilesJsonParser::loadProfiles();
    for(int i=0; i<profiles->count(); ++i)
    {
        PrintProfile* profile = (*profiles)[i];
        _printProfileManager->addProfile(profile);

        item = new QStandardItem(profile->profileName());
        item->setEditable( false );
        if( !i ) {
            firstItem=item;
            _printProfileManager->setActiveProfile(profile->profileName());
        }

        _model->setItem( i, 0, item );
    }

    if( firstItem )
        _profilesList->setCurrentIndex( _model->indexFromItem(firstItem) );

    delete profiles;
}

void ProfilesTab::importParams_clicked(bool)
{
    Window* w = App::mainWindow();
    QRect r = w->geometry();

    QMessageBox msgBox;
    msgBox.setText("Are You sure to import all profiles from USB memory stick?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.move(r.x()+100, r.y()+100);
    msgBox.setFont(*_fontAwesome);


    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:

            // @todo how to pass checkboxes values? what filename means?
            if(!_printProfileManager->importProfiles(nullptr))
            {
               msgBox.setText("Something went wrong. Make sure memory stick is inserted into USB drive.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }
            else
            {
               msgBox.setText("Import successed.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }
            break;
        default:
            break;
    }
}

void ProfilesTab::exportParams_clicked(bool)
{
    Window* w = App::mainWindow();
    QRect r = w->geometry();

    QMessageBox msgBox;
    msgBox.setText("Are You sure to export all profiles to USB memory stick?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.move(r.x()+100, r.y()+100);
    msgBox.setFont(*_fontAwesome);

    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:

            // @todo how to pass checkboxes values? what filename means?
            if(!_printProfileManager->exportProfiles(nullptr))
            {
               msgBox.setText("Something went wrong. Make sure memory stick is inserted into USB drive.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }
            else
            {
               msgBox.setText("Import successed.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }
            break;
        default:
            break;
    }
}

void ProfilesTab::newProfile_clicked(bool)
{
    Window* w = App::mainWindow();
    QRect r = w->geometry();

    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.move(r.x()+100, r.y()+100);
    msgBox.setFont(*_fontAwesome);

    QInputDialog inputDialog;
    inputDialog.setModal(true);
    inputDialog.move(r.x()+100, r.y()+100);
    inputDialog.setFont(*_fontAwesome);
    inputDialog.setFocus(Qt::FocusReason::ActiveWindowFocusReason);
    inputDialog.setLabelText("Enter a profile name: ");
    int ret = inputDialog.exec();

    QString filename = inputDialog.textValue();
    if (ret && !filename.isEmpty())
    {
        if(!_createNewProfile(filename))
        {
            msgBox.setText("Something went wrong.");
            msgBox.exec();
        }
        else
        {
            msgBox.setText("Profile successfuly added.");
            msgBox.exec();
        }
    }
}

void ProfilesTab::deleteProfile_clicked(bool)
{
    Window* w = App::mainWindow();
    QRect r = w->geometry();

    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.move(r.x()+100, r.y()+100);
    msgBox.setFont(*_fontAwesome);
    msgBox.setText("Are You sure to delete selected profile?");
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            if(!_deletePrintProfile())
            {
               msgBox.setText("Something went wrong.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }
            break;
        default:
            break;
    }
}

void ProfilesTab::loadProfile_clicked(bool)
{
    Window* w = App::mainWindow();
    QRect r = w->geometry();

    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.move(r.x()+100, r.y()+100);
    msgBox.setFont(*_fontAwesome);
    msgBox.setText("Are You sure to load selected profile?");
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            if(!_loadPrintProfile())
            {
               msgBox.setText("Something went wrong.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }
            break;
        default:
            break;
    }
}

bool ProfilesTab::_createNewProfile(QString profileName)
{
    PrintProfile* printProfile = (PrintProfile*)_printProfileManager->activeProfile();
    PrintProfile* profileCopy = printProfile->copy();

    profileCopy->setProfileName(profileName);
    _printProfileManager->addProfile(profileCopy);

    QStandardItem* item = new QStandardItem(profileName);
    item->setEditable(false);

    _model->appendRow(item);

    ProfilesJsonParser::saveProfiles(_printProfileManager->profiles());

    return true;
}

bool ProfilesTab::_deletePrintProfile()
{
    QModelIndex index = _profilesList->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    _printProfileManager->removeProfile(itemText);

    _model->removeRows(index.row(), 1);
    ProfilesJsonParser::saveProfiles(_printProfileManager->profiles());
}

bool ProfilesTab::_loadPrintProfile()
{
    QModelIndex index = _profilesList->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    return _printProfileManager->setActiveProfile(itemText);
}
