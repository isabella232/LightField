#include "pch.h"

#include "profilestab.h"

ProfilesTab::ProfilesTab( QWidget* parent ): TabBase( parent ) {

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
