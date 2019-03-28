#ifndef __UISTATE_H__
#define __UISTATE_H__

enum class UiState {
    SelectStarted,
    SelectCompleted,
    SliceStarted,
    SliceCompleted,
    PrintStarted,
    PrintCompleted,
};

char const* ToString( UiState const value );

#endif // !__UISTATE_H__
