#ifndef __VERSION_H__
#define __VERSION_H__

enum class BuildType {
    unknown,
    Release,
    Debug,
};

char      extern const* LIGHTFIELD_VERSION_STRING;
unsigned  extern const  LIGHTFIELD_VERSION_MAJOR;
unsigned  extern const  LIGHTFIELD_VERSION_MINOR;
unsigned  extern const  LIGHTFIELD_VERSION_TEENY;
unsigned  extern const  LIGHTFIELD_VERSION_CODE;

BuildType extern const  LIGHTFIELD_VERSION_BUILD_TYPE;

inline constexpr unsigned MakeVersionCode( int const major, int const minor, int const teeny ) {
    return ( major << 24u ) | ( minor << 16u ) | ( teeny << 8u );
}

inline constexpr void DecodeVersionCode( unsigned const versionCode, int& major, int& minor, int& teeny ) {
    major = static_cast<int>( ( versionCode >> 24u ) & 0xFFu );
    minor = static_cast<int>( ( versionCode >> 16u ) & 0xFFu );
    teeny = static_cast<int>( ( versionCode >>  8u ) & 0xFFu );
}

#endif // __VERSION_H__
