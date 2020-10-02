#ifndef __PROFILESTAB_H__
#define __PROFILESTAB_H__

#include <QtCore>
#include <QtWidgets>
#include "tabbase.h"
#include "printprofilemanager.h"
#include "usbmountmanager.h"

class ProfilesTab: public TabBase
{
    Q_OBJECT

public:

    ProfilesTab(QWidget* parent = nullptr);
    virtual ~ProfilesTab() override;
    virtual TabIndex tabIndex() const override
    {
        return TabIndex::Profiles;
    }


protected:
    virtual void _connectUsbMountManager() override;
    void _filesystemMounted(UsbDevice const &dev, bool writable);
    void _filesystemUnmounted(QString const& mountPoint);
    virtual void _connectPrintManager() override;

private:
    QString _usbMountPoint { "" };
    QPushButton* _importParams { new QPushButton("Import") };
    QPushButton* _exportParams { new QPushButton("Export") };
    QPushButton* _newProfile { new QPushButton("Create profile") };
    QPushButton* _renameProfile { new QPushButton("Rename profile") };
    QPushButton* _overwriteProfile { new QPushButton("Save profile") };
    QPushButton* _deleteProfile { new QPushButton("Delete selected") };
    QPushButton* _loadProfile { new QPushButton("Load selected") };
    QCheckBox* _cpyProfilesUsb { new QCheckBox("Copy profiles from/to USB") };
    QCheckBox* _cpyStlFilesUsb { new QCheckBox("Copy STL files from/to USB") };
    QListView* _profilesList { new QListView };
    QStandardItemModel* _model { new QStandardItemModel };
    QFont* _fontAwesome;
    void _setupProfilesList();
    bool _createNewProfile(QString profileName);
    bool _renamePProfile(QString profileName);
    void _updateProfile();
    void _deletePrintProfile();
    void _loadPrintProfile();
    void _enableButtonProfile(bool enabled, const PrintProfile& selected);
    void _usbRemounted(UsbDevice const &dev, bool succeeded);
    void _setEnabled(bool enabled);
    void _activeProfileChanged(QSharedPointer<PrintProfile> newProfile);
    virtual void _connectPrintProfileManager() override;

public slots:
    virtual void tab_uiStateChanged(TabIndex const sender, UiState const state) override;
    virtual void printJobChanged() override;

    void importParamsClicked(bool);
    void exportParamsClicked(bool);
    void newProfileClicked(bool);
    void renamePProfileClicked(bool);
    void updateProfileClicked(bool);
    void deleteProfileClicked(bool);
    void loadProfileClicked(bool);
    void itemClicked(const QModelIndex& index);

    void printManager_printStarting();
    void printManager_printComplete(bool const success);
    void printManager_printAborted();

    void loadProfiles();

};

#endif //!__PROFILESTAB_H__
