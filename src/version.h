#ifndef __VERSION_H__
#define __VERSION_H__

enum class BuildType {
    Debug,
    Release,
};

char const* VERSION_STRING = "1.0.1";
int  const  VERSION_MAJOR  = 1;
int  const  VERSION_MINOR  = 0;
int  const  VERSION_TEENY  = 1;

#if defined _DEBUG
BuildType const VERSION_BUILD_TYPE = BuildType::Debug;
#else
BuildType const VERSION_BUILD_TYPE = BuildType::Release;
#endif

#endif // __VERSION_H__
