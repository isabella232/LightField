#include "pch.h"

#include "inputdialog.h"
#include "profilestab.h"
#include "printmanager.h"
#include "profilesjsonparser.h"
#include "usbmountmanager.h"
#include "window.h"

#include <dirent.h>
#include <iostream>

ProfilesTab::ProfilesTab(QWidget* parent): InitialShowEventMixin<ProfilesTab, TabBase>(parent)
{
    auto origFont    = font();
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome", LargeFontSize );
    _fontAwesome = new QFont(fontAwesome);

    QGroupBox* cpyProfilesUsbBox = new QGroupBox();
    cpyProfilesUsbBox->setLayout(WrapWidgetsInVBox(_cpyProfilesUsb, nullptr));
    cpyProfilesUsbBox->setContentsMargins( { } );

    QGroupBox* cpyStlFilesUsbBox = new QGroupBox();
    cpyStlFilesUsbBox->setLayout(WrapWidgetsInVBox(_cpyStlFilesUsb, nullptr));
    cpyStlFilesUsbBox->setContentsMargins( { } );


    _importParams->setEnabled(false);
    _importParams->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _exportParams->setEnabled(false);
    _exportParams->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _newProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _renameProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _overwriteProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _deleteProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _loadProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Default disabled all button  no profile is selected
    _overwriteProfile->setEnabled(false);
    _deleteProfile->setEnabled(false);
    _loadProfile->setEnabled(false);
    _renameProfile->setEnabled(false);
    _setupProfilesList();

    QObject::connect(_importParams, &QPushButton::clicked, this,&ProfilesTab::importParamsClicked);
    QObject::connect(_exportParams, &QPushButton::clicked, this, &ProfilesTab::exportParamsClicked);
    QObject::connect(_newProfile, &QPushButton::clicked, this, &ProfilesTab::newProfileClicked);
    QObject::connect(_renameProfile, &QPushButton::clicked, this, &ProfilesTab::renamePProfileClicked);
    QObject::connect(_overwriteProfile, &QPushButton::clicked, this, &ProfilesTab::updateProfileClicked);
    QObject::connect(_deleteProfile, &QPushButton::clicked, this, &ProfilesTab::deleteProfileClicked);
    QObject::connect(_loadProfile, &QPushButton::clicked, this, &ProfilesTab::loadProfileClicked);
    QObject::connect(_profilesList, &QListView::clicked, this, &ProfilesTab::itemClicked);

    setLayout(
        WrapWidgetsInHBox(
              WrapWidgetsInVBox(
                   _exportParams,
                   _importParams,
                   _loadProfile,
                   _deleteProfile,
                   _newProfile,
                   _renameProfile,
                   _overwriteProfile
              ),
              _profilesList
        )
    );
}

ProfilesTab::~ProfilesTab()
{
    /*empty*/
}

void ProfilesTab::_initialShowEvent(QShowEvent* event) {
    auto activeProfile = _printProfileManager->activeProfile();

    const QAbstractItemModel *model = _profilesList->model();
    const QModelIndexList indexes = model->match(
        model->index(0,0),
        Qt::DisplayRole,
        QVariant(activeProfile->profileName()),
        1, // first hit only
        Qt::MatchExactly
    );

    if(indexes.length() > 0) {
        _profilesList->setCurrentIndex(indexes.at(0));
        itemClicked(indexes.at(0));
    }

    event->accept( );
}

void ProfilesTab::itemClicked(const QModelIndex& index)
{
    QString name = index.data(Qt::DisplayRole).toString();
    auto profile = _printProfileManager->getProfile(name);

    _enableButtonProfile(true, *profile);
}


void ProfilesTab::_enableButtonProfile(bool enabled, const PrintProfile& selected)
{
    bool enable = !selected.isActive() && !selected.isDefault();

    if (!_printManager->isRunning()) {
        _overwriteProfile->setEnabled(enabled);
        _deleteProfile->setEnabled(enabled && enable);
        _loadProfile->setEnabled(enabled);
        _renameProfile->setEnabled(enabled && enable);
        _newProfile->setEnabled(enabled);
    }
}

void ProfilesTab::tab_uiStateChanged(TabIndex const sender, UiState const state)
{
    debug( "+ ProfilesTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch (_uiState) {
    default:
        break;
    }
}

void ProfilesTab::_setupProfilesList()
{
    QItemSelectionModel* selectionModel = new QItemSelectionModel(_model);
    _profilesList->setModel(_model);
    _profilesList->setSelectionModel(selectionModel);
    _profilesList->setVisible(true);
    _profilesList->setSelectionBehavior(QAbstractItemView::SelectRows);
    _profilesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ProfilesTab::_connectPrintProfileManager()
{
    connect(_printProfileManager, &PrintProfileManager::reloadProfiles, this,
        &ProfilesTab::loadProfiles);
    connect(_printProfileManager, &PrintProfileManager::activeProfileChanged, this,
        &ProfilesTab::loadProfiles);

    _printProfileManager->reload();
}

void ProfilesTab::_usbRemounted(const bool succeeded, const bool writable)
{
    QMessageBox msgBox { this };
    QObject::disconnect(_usbMountManager, &UsbMountManager::filesystemRemounted, this,
        &ProfilesTab::_usbRemounted);

    if (!succeeded) {
        msgBox.setText("Could not remount USB stick. Make sure memory stick is inserted into USB drive.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    if (!_printProfileManager->exportProfiles(_usbMountPoint)) {
        msgBox.setText("Could not export profiles. Make sure memory stick is inserted into USB drive.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        _usbMountManager->remount(false);
        return;
    }

    msgBox.setText("Export succeeded.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    _usbMountManager->remount(false);
}

void ProfilesTab::importParamsClicked(bool)
{
    QMessageBox msgBox { this };
    // <nobr> because when two line text causes wrong value of sizeHint().width() and sizeHint().height()
    msgBox.setText("<center><nobr>Are You sure to import all profiles <br>from USB memory stick?</nobr></center>");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            if(!_printProfileManager->importProfiles(_usbMountPoint)) {
               msgBox.setText("<center><nobr>Something went wrong. Make sure memory <br>stick is inserted into USB drive.</nobr></center>");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            } else {
               msgBox.setText("Import succeeded.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }

            loadProfiles();
            break;

        default:
            break;
    }
}

void ProfilesTab::exportParamsClicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setText("<center><nobr>Are You sure to export all profiles <br>to USB memory stick?</nobr></center>");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            QObject::connect(_usbMountManager, &UsbMountManager::filesystemRemounted, this,
                &ProfilesTab::_usbRemounted);
            _usbMountManager->remount(true);
            break;
        default:
            break;
    }
}

void ProfilesTab::newProfileClicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Ok);

    InputDialog* inputDialog = new InputDialog(QString("Entry profile name: "));
    int ret = inputDialog->exec();

    QString filename = inputDialog->getValue().trimmed();

    if (ret && filename.isEmpty()) {
        msgBox.setText("Profile name cannot be empty");
        msgBox.exec();
        return;
    }

    if (ret && !filename.isEmpty()) {
        if (!_createNewProfile(filename)) {
            msgBox.setText("Something went wrong.");
            msgBox.exec();
        } else {
            msgBox.setText("Profile successfuly added.");
            msgBox.exec();
        }
    }
}

void ProfilesTab::renamePProfileClicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Ok);
    InputDialog* inputDialog = new InputDialog(QString("Entry profile name: "));
    int ret = inputDialog->exec();

    QString filename = inputDialog->getValue();
    if (ret && !filename.isEmpty())
    {
        if(_renamePProfile(filename)) {
            msgBox.setText("Profile successfuly renamed.");
            msgBox.exec();
        }
    }
}

void ProfilesTab::updateProfileClicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText("Are You sure to update selected profile?");
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            _updateProfile();
            break;
        default:
            break;
    }
}

void ProfilesTab::deleteProfileClicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText("Are You sure to delete selected profile?");
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            _deletePrintProfile();
            break;
        default:
            break;
    }
}

void ProfilesTab::loadProfileClicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText("Are You sure to load selected profile. All local changes will discarded?");
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            _loadPrintProfile();
            break;
        default:
            break;
    }
}

bool ProfilesTab::_createNewProfile(QString profileName)
{
    QSharedPointer<PrintProfile> printProfile = _printProfileManager->activeProfile();
    QSharedPointer<PrintProfile> profileCopy { new PrintProfile(*printProfile) };

    profileCopy->setProfileName(profileName);
    _printProfileManager->addProfile(profileCopy);
    return true;
}

bool ProfilesTab::_renamePProfile(QString profileName)
{
    QModelIndex index = _profilesList->currentIndex();
    QMessageBox msgBox { this };
    QString oldName = index.data(Qt::DisplayRole).toString();

    auto profile = _printProfileManager->getProfile(oldName);
    if (profile.isNull()) {
        msgBox.setText("Profile not found");
        msgBox.exec();
        return false;
    }

    try {
        _printProfileManager->renameProfile(oldName, profileName);
    } catch (std::runtime_error& e) {
        msgBox.setText(e.what());
        msgBox.exec();
        return false;
    }

    _model->setData(index, QVariant(profileName));

    return true;
}

void ProfilesTab::_updateProfile()
{
    QMessageBox msgBox { this };
    QModelIndex index = _profilesList->currentIndex();
    QString profileName = index.data(Qt::DisplayRole).toString();
    auto activeProfile = _printProfileManager->activeProfile();

    if (profileName.isEmpty()) {
        msgBox.setText("Please select a profile to update");
        msgBox.exec();
        return;
    }

    auto profile = _printProfileManager->getProfile(profileName);
    if (profile.isNull()) {
        msgBox.setText("Profile not found");
        msgBox.exec();
        return;
    }

    if (profile->isDefault()) {
        msgBox.setText("Cannot overwrite default profile");
        msgBox.exec();
        return;
    }

    profile->setBaseLayerCount(_printJob->baseSlices.layerCount);
    _printProfileManager->saveProfile(profileName);
    profile->debugPrint();
}

void ProfilesTab::_deletePrintProfile()
{
    QMessageBox msgBox { this };
    QModelIndex index = _profilesList->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();

    try {
        _printProfileManager->removeProfile(itemText);
    } catch (const std::exception &ex) {
        msgBox.setText(ex.what());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void ProfilesTab::_loadPrintProfile()
{
    QMessageBox msgBox { this };
    QModelIndex index = _profilesList->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    _printProfileManager->loadProfile(itemText);

    try {
        _printProfileManager->setActiveProfile(itemText);
    } catch (const std::exception &ex) {
        msgBox.setText(ex.what());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    _printJob->setPrintProfile(_printProfileManager->activeProfile());
}

void ProfilesTab::loadProfiles()
{
    QStandardItem* item = nullptr;
    auto origFont = font();
    auto fontAwesome = ModifyFont(origFont, "FontAwesome", LargeFontSize);
    auto boldFont = ModifyFont(fontAwesome, QFont::Bold);

    _model->removeRows(0, _model->rowCount());

    foreach (const auto profile, _printProfileManager->profiles()) {
        item = new QStandardItem(profile->profileName());
        item->setEditable(false);
        item->setFont(profile->isActive() ? boldFont : fontAwesome);

        _profilesList->setCurrentIndex(_model->indexFromItem(item));
        _model->appendRow(item);
    }
}

void ProfilesTab::_connectUsbMountManager()
{
    QObject::connect(_usbMountManager, &UsbMountManager::filesystemMounted, this,
        &ProfilesTab::_filesystemMounted);
    QObject::connect(_usbMountManager, &UsbMountManager::filesystemUnmounted, this,
        &ProfilesTab::_filesystemUnmounted);
}

void ProfilesTab::_filesystemMounted(const QString& mountPoint)
{
    debug( "+ ProfilesTab::_filesystemMounted: mount point '%s'\n", mountPoint.toUtf8().data());
    _importParams->setEnabled(true);
    _exportParams->setEnabled(true);
    _usbMountPoint = mountPoint;
}

void ProfilesTab::_filesystemUnmounted(const QString& mountPoint)
{
    debug("+ ProfilesTab::_filesystemUnmounted: mount point '%s'\n", mountPoint.toUtf8().data());
    _importParams->setEnabled(false);
    _exportParams->setEnabled(false);
    _usbMountPoint = "";
}

void ProfilesTab::_setEnabled(bool enabled)
{
    _loadProfile->setEnabled(enabled);
    _deleteProfile->setEnabled(enabled);
    _newProfile->setEnabled(enabled);
    _renameProfile->setEnabled(enabled);
    _overwriteProfile->setEnabled(enabled);
}

void ProfilesTab::_connectPrintManager()
{

    if (_printManager) {
        QObject::connect(_printManager, &PrintManager::printStarting, this,
           &ProfilesTab::printManager_printStarting);
        QObject::connect(_printManager, &PrintManager::printComplete, this,
           &ProfilesTab::printManager_printComplete);
        QObject::connect(_printManager, &PrintManager::printAborted, this,
           &ProfilesTab::printManager_printAborted);
    }
}

void ProfilesTab::printManager_printStarting()
{

    debug("+ ProfilesTab::printManager_printStarting\n");

    _setEnabled(false);

    update();
}

void ProfilesTab::printManager_printComplete(const bool success)
{

    debug("+ ProfilesTab::printManager_printComplete\n");
    (void)success;

    QModelIndex index = _profilesList->currentIndex();
    QString indexName = index.data(Qt::DisplayRole).toString();

    auto profile = _printProfileManager->getProfile(indexName);
    if (!profile)
        profile = _printProfileManager->activeProfile();

    _enableButtonProfile(true, *profile);

    update();
}

void ProfilesTab::printManager_printAborted()
{

    debug("+ ProfilesTab::printManager_printAborted\n");

    QModelIndex index = _profilesList->currentIndex();
    QString indexName = index.data(Qt::DisplayRole).toString();

    auto profile = _printProfileManager->getProfile(indexName);
    if (!profile)
        profile = _printProfileManager->activeProfile();

    _enableButtonProfile(true, *profile);

    update();
}
