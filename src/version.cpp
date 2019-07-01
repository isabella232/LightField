#include "pch.h"

#include "version.h"

char      const* LIGHTFIELD_VERSION_STRING     = "1.0.2";
unsigned  const  LIGHTFIELD_VERSION_MAJOR      = 1;
unsigned  const  LIGHTFIELD_VERSION_MINOR      = 0;
unsigned  const  LIGHTFIELD_VERSION_TEENY      = 2;
unsigned  const  LIGHTFIELD_VERSION_CODE       = MakeVersionCode( LIGHTFIELD_VERSION_MAJOR, LIGHTFIELD_VERSION_MINOR, LIGHTFIELD_VERSION_TEENY );

#if defined _DEBUG
BuildType const  LIGHTFIELD_VERSION_BUILD_TYPE = BuildType::Debug;
#else
BuildType const  LIGHTFIELD_VERSION_BUILD_TYPE = BuildType::Release;
#endif
