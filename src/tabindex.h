#ifndef __TABINDEX_H__
#define __TABINDEX_H__

enum class TabIndex {
    File,
    Prepare,
    Print,
    Status,
    Advanced,
    System,
};

inline int operator+( TabIndex const value ) { return static_cast<int>( value ); }

char const* ToString( TabIndex const value );

#endif // !__TABINDEX_H__
