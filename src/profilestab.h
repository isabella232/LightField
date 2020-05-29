#ifndef __PROFILESTAB_H__
#define __PROFILESTAB_H__

#include <QtCore>
#include <QtWidgets>
#include "tabbase.h"
#include "printprofilemanager.h"

class ProfilesTab: public TabBase
{
    Q_OBJECT

public:

    ProfilesTab( QWidget* parent = nullptr );
    virtual ~ProfilesTab( ) override;

    virtual TabIndex tabIndex( ) const override { return TabIndex::Profiles; }

    void setPrintProfileManager( PrintProfileManager* printProfileManager );

protected:
    virtual void _connectUsbMountManager() override;
    void _filesystemMounted(QString const& mountPoint);
    void _filesystemUnmounted(QString const& mountPoint);

private:
    PrintProfileManager* _printProfileManager;
    QString _usbMountPoint { "" };

    QPushButton*        _importParams                    { new QPushButton("Import")              };
    QPushButton*        _exportParams                    { new QPushButton("Export")              };
    QPushButton*        _newProfile                      { new QPushButton("Create profile")      };
    QPushButton*        _renameProfile                   { new QPushButton("Rename profile")      };
    QPushButton*        _overwriteProfile                { new QPushButton("Save profile")        };
    QPushButton*        _deleteProfile                   { new QPushButton("Delete selected")     };
    QPushButton*        _loadProfile                     { new QPushButton("Load selected")       };
    QCheckBox*          _cpyProfilesUsb                  { new QCheckBox("Copy profiles from/to USB")  };
    QCheckBox*          _cpyStlFilesUsb                  { new QCheckBox("Copy STL files from/to USB") };
    QListView*          _profilesList                    { new QListView                          };
    QStandardItemModel* _model                           { new QStandardItemModel                 };
    QFont*              _fontAwesome;
    void _setupProfilesList(QFont font);
    bool _createNewProfile(QString profileName);
    bool _renamePProfile(QString profileName);
    bool _updateProfile();
    bool _deletePrintProfile();
    bool _loadPrintProfile();
    void _enableButtonProfile(bool enabled);
    void _usbRemounted(const bool succeeded, const bool writable);
    void _setEnabled( bool enabled );

public slots:
    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void importParams_clicked(bool);
    void exportParams_clicked(bool);
    void newProfile_clicked(bool);
    void renamePProfile_clicked(bool);
    void updateProfile_clicked(bool);
    void deleteProfile_clicked(bool);
    void loadProfile_clicked(bool);
    void itemClicked(const QModelIndex &index);

    void loadProfiles( PrintProfileCollection* profiles );

};

#endif //!__PROFILESTAB_H__
