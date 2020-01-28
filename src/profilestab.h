#ifndef __PROFILESTAB_H__
#define __PROFILESTAB_H__

#include "tabbase.h"

#include "printprofilemanager.h"

class ProfilesTab: public TabBase {

    Q_OBJECT

public:

    ProfilesTab( QWidget* parent = nullptr );
    virtual ~ProfilesTab( ) override;

    virtual TabIndex tabIndex( ) const override { return TabIndex::Profiles; }

protected:

private:

signals:
    ;

public slots:
    ;

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

protected slots:
    ;

private slots:
    ;

};

#endif //!__PROFILESTAB_H__
