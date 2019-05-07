#include "pch.h"

#include "version.h"

char const* VERSION_STRING = "1.0.1";
int  const  VERSION_MAJOR = 1;
int  const  VERSION_MINOR = 0;
int  const  VERSION_TEENY = 1;

#if defined _DEBUG
BuildType const VERSION_BUILD_TYPE = BuildType::Debug;
#else
BuildType const VERSION_BUILD_TYPE = BuildType::Release;
#endif
