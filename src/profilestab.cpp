#include "pch.h"

#include "inputdialog.h"
#include "profilestab.h"
#include "profilesjsonparser.h"
#include "usbmountmanager.h"
#include "window.h"

#include <dirent.h>
#include <iostream>

class ProfileItemDelegate: public QStyledItemDelegate
{
    ProfileItemDelegate(QObject* parent = nullptr): QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index) const
    {

    }
};

ProfilesTab::ProfilesTab(QWidget* parent): TabBase(parent)
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


    _cpyProfilesUsb->setFont(fontAwesome);
    _cpyStlFilesUsb->setFont(fontAwesome);

    _importParams->setFont(fontAwesome);
    _importParams->setEnabled(false);
    _importParams->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _exportParams->setFont(fontAwesome);
    _exportParams->setEnabled(false);
    _exportParams->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _newProfile->setFont(fontAwesome);
    _newProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _renameProfile->setFont(fontAwesome);
    _renameProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _overwriteProfile->setFont(fontAwesome);
    _overwriteProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _deleteProfile->setFont(fontAwesome);
    _deleteProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _loadProfile->setFont(fontAwesome);
    _loadProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Default disabled all button  no profile is selected
    _enableButtonProfile(false);
    _setupProfilesList(fontAwesome);

    QObject::connect(_importParams, &QPushButton::clicked, this, &ProfilesTab::importParams_clicked);
    QObject::connect(_exportParams, &QPushButton::clicked, this, &ProfilesTab::exportParams_clicked);
    QObject::connect(_newProfile, &QPushButton::clicked, this, &ProfilesTab::newProfile_clicked);
    QObject::connect(_renameProfile, &QPushButton::clicked, this, &ProfilesTab::renamePProfile_clicked);
    QObject::connect(_overwriteProfile, &QPushButton::clicked, this, &ProfilesTab::updateProfile_clicked);
    QObject::connect(_deleteProfile, &QPushButton::clicked, this, &ProfilesTab::deleteProfile_clicked);
    QObject::connect(_loadProfile, &QPushButton::clicked, this, &ProfilesTab::loadProfile_clicked);
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

ProfilesTab::~ProfilesTab( ) {
    /*empty*/
}


void ProfilesTab::itemClicked(const QModelIndex& index)
{
    (void)index;
    _enableButtonProfile(true);
}


void ProfilesTab::_enableButtonProfile(bool enabled)
{
    _overwriteProfile->setEnabled(enabled);
    _deleteProfile->setEnabled(enabled);
    _loadProfile->setEnabled(enabled);
    _renameProfile->setEnabled(enabled);
}

void ProfilesTab::tab_uiStateChanged(TabIndex const sender, UiState const state)
{
    debug( "+ ProfilesTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch (_uiState) {
    case UiState::PrintStarted:
        _setEnabled(false);
        break;
    case UiState::PrintCompleted:
        _setEnabled(true);
        break;
    default:
        break;
    }
}

void ProfilesTab::_setupProfilesList(QFont font)
{
    QItemSelectionModel* selectionModel = new QItemSelectionModel(_model);
    _profilesList->setModel(_model);
    _profilesList->setSelectionModel(selectionModel);
    _profilesList->setFont(font);
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
    msgBox.setFont(*_fontAwesome);

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

void ProfilesTab::importParams_clicked(bool)
{
    QMessageBox msgBox { this };
    // <nobr> because when two line text causes wrong value of sizeHint().width() and sizeHint().height()
    msgBox.setText("<center><nobr>Are You sure to import all profiles <br>from USB memory stick?</nobr></center>");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setFont(*_fontAwesome);
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

void ProfilesTab::exportParams_clicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setText("<center><nobr>Are You sure to export all profiles <br>to USB memory stick?</nobr></center>");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setFont(*_fontAwesome);
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

void ProfilesTab::newProfile_clicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setFont(*_fontAwesome);

    InputDialog* inputDialog = new InputDialog(QString("Entry profile name: "));
    int ret = inputDialog->exec();

    QString filename = inputDialog->getValue();

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

void ProfilesTab::renamePProfile_clicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setFont(*_fontAwesome);
    InputDialog* inputDialog = new InputDialog(QString("Entry profile name: "));
    int ret = inputDialog->exec();

    QString filename = inputDialog->getValue();
    if (ret && !filename.isEmpty())
    {
        if(!_renamePProfile(filename)) {
            msgBox.setText("Something went wrong.");
            msgBox.exec();
        } else {
            msgBox.setText("Profile successfuly renamed.");
            msgBox.exec();
        }
    }
}

void ProfilesTab::updateProfile_clicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setFont(*_fontAwesome);
    msgBox.setText("Are You sure to update selected profile?");
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            if (!_updateProfile()) {
               msgBox.setText("Something went wrong.");
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.exec();
            }
            break;
        default:
            break;
    }
}

void ProfilesTab::deleteProfile_clicked(bool)
{
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
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
    QMessageBox msgBox { this };
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setFont(*_fontAwesome);
    msgBox.setText("Are You sure to load selected profile. All local changes will discard?");
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
    QSharedPointer<PrintProfile> printProfile = _printProfileManager->activeProfile();
    QSharedPointer<PrintProfile> profileCopy { new PrintProfile(*printProfile) };

    profileCopy->setProfileName(profileName);
    _printProfileManager->addProfile(profileCopy);
    return true;
}

bool ProfilesTab::_renamePProfile(QString profileName)
{
    QModelIndex index = _profilesList->currentIndex();

    QString oldName = index.data(Qt::DisplayRole).toString();
    _model->setData(index, QVariant(profileName));

    auto profile = _printProfileManager->getProfile(profileName);
    profile->setProfileName(profileName);
    _printProfileManager->saveProfiles();

    return true;
}

bool ProfilesTab::_updateProfile()
{
    QModelIndex index = _profilesList->currentIndex();
    QString profileName = index.data(Qt::DisplayRole).toString();

    auto profile = _printProfileManager->getProfile(profileName);
    auto activeProfile = _printProfileManager->activeProfile();

    profile->setBaseLayerCount(activeProfile->baseLayerCount());

    PrintParameters baseParams = activeProfile->baseLayerParameters();
    PrintParameters newBaseParams;

    newBaseParams.setPumpUpDistance( baseParams.pumpUpDistance() );
    newBaseParams.setPumpUpVelocity( baseParams.pumpUpVelocity_Effective() );
    newBaseParams.setPumpUpPause( baseParams.pumpUpPause() );
    newBaseParams.setPumpDownVelocity( baseParams.pumpDownVelocity_Effective() );
    newBaseParams.setPumpDownPause( baseParams.pumpDownPause() );
    newBaseParams.setNoPumpUpVelocity( baseParams.noPumpUpVelocity() );
    newBaseParams.setPumpEveryNthLayer( baseParams.pumpEveryNthLayer() );
    newBaseParams.setLayerThickness( baseParams.layerThickness() );
    newBaseParams.setLayerExposureTime( baseParams.layerExposureTime() );
    newBaseParams.setPowerLevel( baseParams.powerLevel() );
    newBaseParams.setPumpingEnabled( baseParams.isPumpingEnabled() );

    profile->setBaseLayerParameters( newBaseParams );

    PrintParameters bodyParams = activeProfile->bodyLayerParameters();
    PrintParameters newBodyParams;

    newBodyParams.setPumpUpDistance( bodyParams.pumpUpDistance() );
    newBodyParams.setPumpUpVelocity( bodyParams.pumpUpVelocity_Effective() );
    newBodyParams.setPumpUpPause( bodyParams.pumpUpPause() );
    newBodyParams.setPumpDownVelocity( bodyParams.pumpDownVelocity_Effective() );
    newBodyParams.setPumpDownPause( bodyParams.pumpDownPause() );
    newBodyParams.setNoPumpUpVelocity( bodyParams.noPumpUpVelocity() );
    newBodyParams.setPumpEveryNthLayer( bodyParams.pumpEveryNthLayer() );
    newBodyParams.setLayerThickness( bodyParams.layerThickness() );
    newBodyParams.setLayerExposureTime( bodyParams.layerExposureTime() );
    newBodyParams.setPowerLevel( bodyParams.powerLevel() );
    newBodyParams.setPumpingEnabled( bodyParams.isPumpingEnabled() );

    profile->setBodyLayerParameters( newBodyParams );

    ProfilesJsonParser::saveProfiles(_printProfileManager->profiles());
    profile->debugPrint();

    return true;
}

bool ProfilesTab::_deletePrintProfile()
{
    QModelIndex index = _profilesList->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    if( _printProfileManager->removeProfile(itemText) ) {
        _model->removeRows(index.row(), 1);

        _enableButtonProfile(false); // Because no item is selected after deleted

        return true;
    } else {
        return false;
    }
}

bool ProfilesTab::_loadPrintProfile()
{
    QModelIndex index = _profilesList->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    return _printProfileManager->setActiveProfile(itemText);
}

void ProfilesTab::loadProfiles()
{
    QStandardItem* item = nullptr;

    _model->removeRows(0, _model->rowCount());

    foreach (QSharedPointer<PrintProfile> profile, _printProfileManager->profiles()) {
        item = new QStandardItem(profile->profileName());
        item->setEditable(false);  
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

void ProfilesTab::_setEnabled( bool enabled ) {
    _loadProfile->setEnabled( enabled );
    _deleteProfile->setEnabled( enabled );
    _newProfile->setEnabled( enabled );
    _renameProfile->setEnabled( enabled );
    _overwriteProfile->setEnabled( enabled );
}

void ProfilesTab::_activeProfileChanged(QSharedPointer<PrintProfile> newProfile)
{
    if (!_printJob.isNull())
        _printJob->printProfile = newProfile;
}
