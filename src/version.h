#ifndef __VERSION_H__
#define __VERSION_H__

enum class BuildType {
    unknown,
    Release,
    Debug,
};

inline constexpr unsigned MakeVersionCode( unsigned const major, unsigned const minor, unsigned const teeny, unsigned const build = 0 ) {
    return ( ( major & 0xFFu ) << 24u ) | ( ( minor & 0xFFu ) << 16u ) | ( ( teeny & 0xFFu ) << 8u ) | ( build & 0xFFu );
}

inline constexpr void DecodeVersionCode( unsigned const versionCode, int& major, int& minor, int& teeny, int& build ) {
    major = static_cast<int>( ( versionCode >> 24u ) & 0xFFu );
    minor = static_cast<int>( ( versionCode >> 16u ) & 0xFFu );
    teeny = static_cast<int>( ( versionCode >>  8u ) & 0xFFu );
    build = static_cast<int>(   versionCode          & 0xFFu );
}

char      const* LIGHTFIELD_VERSION_STRING __attribute__(( weak )) = "1.0.7.0";
unsigned  const  LIGHTFIELD_VERSION_MAJOR                          = 1;
unsigned  const  LIGHTFIELD_VERSION_MINOR                          = 0;
unsigned  const  LIGHTFIELD_VERSION_TEENY                          = 7;
unsigned  const  LIGHTFIELD_VERSION_BUILD                          = 0;
unsigned  const  LIGHTFIELD_VERSION_CODE                           = MakeVersionCode( LIGHTFIELD_VERSION_MAJOR, LIGHTFIELD_VERSION_MINOR, LIGHTFIELD_VERSION_TEENY, LIGHTFIELD_VERSION_BUILD );

#if defined _DEBUG
BuildType const  LIGHTFIELD_VERSION_BUILD_TYPE                     = BuildType::Debug;
#else
BuildType const  LIGHTFIELD_VERSION_BUILD_TYPE                     = BuildType::Release;
#endif

#endif // __VERSION_H__
