QT                        += core gui widgets xml
CONFIG                    += c++1z precompile_header
TARGET                     = lf
TEMPLATE                   = app
PRECOMPILED_HEADER         = ../src/pch.h

QMAKE_CXXFLAGS_RELEASE    -= -O2
QMAKE_CXXFLAGS_RELEASE    += -O3 -DNDEBUG -Wall -Wextra -Winvalid-pch -Wno-unused-result -Wno-class-memaccess
QMAKE_CXXFLAGS_DEBUG      += -Og -D_DEBUG -Wall -Wextra -Winvalid-pch -Wno-unused-result -Wno-class-memaccess

contains(QMAKE_HOST.arch, armv7l):{
    QMAKE_CXXFLAGS         = -pipe -march=armv8-a -mtune=cortex-a72 -mfpu=crypto-neon-fp-armv8 -mfloat-abi=hard

    #QMAKE_LIBS_EGL         = -lbrcmEGL -lbrcmGLESv2
    #QMAKE_LIBS_OPENGL_ES2  = -lbrcmGLESv2 -lbrcmEGL
    #QMAKE_LIBS_OPENVG      = -lbcrmEGL -lbrcmOpenVG -lbrcmGLESv2

    #QMAKE_LIBS_EGL         = -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib -lbrcmEGL -lbrcmGLESv2
    #QMAKE_LIBS_OPENGL_ES2  = -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib -lbrcmGLESv2 -lbrcmEGL
    #QMAKE_LIBS_OPENVG      = -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib -lbcrmEGL -lbrcmOpenVG -lbrcmGLESv2
}

RESOURCES                 += ../gl/gl.qrc ../images/images.qrc ../text/text.qrc

SOURCES                   +=       \
    ../src/advancedtab.cpp         \
    ../src/app.cpp                 \
    ../src/backdrop.cpp            \
    ../src/canvas.cpp              \
    ../src/constants.cpp           \
    ../src/debug.cpp               \
    ../src/debuglogcopier.cpp      \
    ../src/filecopier.cpp          \
    ../src/filetab.cpp             \
    ../src/gesturelistview.cpp     \
    ../src/glmesh.cpp              \
    ../src/gpgsignaturechecker.cpp \
    ../src/hasher.cpp              \
    ../src/lightfieldstyle.cpp     \
    ../src/loader.cpp              \
    ../src/main.cpp                \
    ../src/mesh.cpp                \
    ../src/pngdisplayer.cpp        \
    ../src/preparetab.cpp          \
    ../src/printmanager.cpp        \
    ../src/printtab.cpp            \
    ../src/processrunner.cpp       \
    ../src/shepherd.cpp            \
    ../src/signalhandler.cpp       \
    ../src/statustab.cpp           \
    ../src/stdiologger.cpp         \
    ../src/strings.cpp             \
    ../src/svgrenderer.cpp         \
    ../src/systemtab.cpp           \
    ../src/tabbase.cpp             \
    ../src/timinglogger.cpp        \
    ../src/upgradekitunpacker.cpp  \
    ../src/upgrademanager.cpp      \
    ../src/upgradeselector.cpp     \
    ../src/usbmountmanager.cpp     \
    ../src/utils.cpp               \
    ../src/window.cpp

HEADERS                   +=       \
    ../src/advancedtab.h           \
    ../src/app.h                   \
    ../src/backdrop.h              \
    ../src/canvas.h                \
    ../src/constants.h             \
    ../src/coordinate.h            \
    ../src/debug.h                 \
    ../src/debuglogcopier.h        \
    ../src/filecopier.h            \
    ../src/filetab.h               \
    ../src/gesturelistview.h       \
    ../src/glerr.h                 \
    ../src/glmesh.h                \
    ../src/gpgsignaturechecker.h   \
    ../src/hasher.h                \
    ../src/initialshoweventmixin.h \
    ../src/lightfieldstyle.h       \
    ../src/loader.h                \
    ../src/mesh.h                  \
    ../src/pngdisplayer.h          \
    ../src/preparetab.h            \
    ../src/printjob.h              \
    ../src/printmanager.h          \
    ../src/printtab.h              \
    ../src/processrunner.h         \
    ../src/shepherd.h              \
    ../src/signalhandler.h         \
    ../src/statustab.h             \
    ../src/stdiologger.h           \
    ../src/strings.h               \
    ../src/svgrenderer.h           \
    ../src/systemtab.h             \
    ../src/tabbase.h               \
    ../src/timinglogger.h          \
    ../src/upgradekitunpacker.h    \
    ../src/upgrademanager.h        \
    ../src/upgradeselector.h       \
    ../src/usbmountmanager.h       \
    ../src/utils.h                 \
    ../src/version.h               \
    ../src/vertex.h                \
    ../src/window.h
