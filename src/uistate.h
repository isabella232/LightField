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

inline int operator+( UiState const value ) { return static_cast<int>( value ); }

char const* ToString( UiState const value );

#endif // !__UISTATE_H__
