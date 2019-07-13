#ifndef __VERSION_H__
#define __VERSION_H__

enum class BuildType {
    unknown,
    Release,
    Debug,
};

inline constexpr unsigned MakeVersionCode( unsigned const major, unsigned const minor, unsigned const teeny ) {
    return ( major << 24u ) | ( minor << 16u ) | ( teeny << 8u );
}

char      const* LIGHTFIELD_VERSION_STRING __attribute__(( weak )) = "1.0.4";
unsigned  const  LIGHTFIELD_VERSION_MAJOR                          = 1;
unsigned  const  LIGHTFIELD_VERSION_MINOR                          = 0;
unsigned  const  LIGHTFIELD_VERSION_TEENY                          = 4;
unsigned  const  LIGHTFIELD_VERSION_CODE                           = MakeVersionCode( LIGHTFIELD_VERSION_MAJOR, LIGHTFIELD_VERSION_MINOR, LIGHTFIELD_VERSION_TEENY );

#if defined _DEBUG
BuildType const  LIGHTFIELD_VERSION_BUILD_TYPE                     = BuildType::Debug;
#else
BuildType const  LIGHTFIELD_VERSION_BUILD_TYPE                     = BuildType::Release;
#endif

#endif // __VERSION_H__
