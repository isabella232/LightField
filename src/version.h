#ifndef __VERSION_H__
#define __VERSION_H__

enum class BuildType {
    Debug,
    Release,
};

char extern const* VERSION_STRING;
int  extern const  VERSION_MAJOR;
int  extern const  VERSION_MINOR;
int  extern const  VERSION_TEENY;

BuildType extern const VERSION_BUILD_TYPE;

#endif // __VERSION_H__
