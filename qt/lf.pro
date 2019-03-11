QT += core gui opengl widgets xml

TARGET = lf
TEMPLATE = app

# Bump optimization up to -O3 in release builds
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES +=                     \
    ../src/debug.cpp           \
    ../src/app.cpp             \
    ../src/main.cpp            \
    ../src/canvas.cpp          \
    ../src/mesh.cpp            \
    ../src/glmesh.cpp          \
    ../src/loader.cpp          \
    ../src/window.cpp          \
    ../src/backdrop.cpp        \
    ../src/shepherd.cpp        \
    ../src/strings.cpp         \
    ../src/printmanager.cpp    \
    ../src/pngdisplayer.cpp    \
    ../src/svgrenderer.cpp     \
    ../src/processrunner.cpp   \
    ../src/signalhandler.cpp   \
    ../src/welcometab.cpp      \
    ../src/selecttab.cpp       \
    ../src/preparetab.cpp      \
    ../src/printtab.cpp        \
    ../src/statustab.cpp       \
    ../src/advancedtab.cpp     \
    ../src/constants.cpp       \
    ../src/utils.cpp           \
    ../src/gesturelistview.cpp \
    ../src/hasher.cpp

HEADERS  += \
    ../src/debug.h           \
    ../src/app.h             \
    ../src/canvas.h          \
    ../src/mesh.h            \
    ../src/glmesh.h          \
    ../src/loader.h          \
    ../src/window.h          \
    ../src/backdrop.h        \
    ../src/shepherd.h        \
    ../src/strings.h         \
    ../src/printmanager.h    \
    ../src/printjob.h        \
    ../src/pngdisplayer.h    \
    ../src/svgrenderer.h     \
    ../src/processrunner.h   \
    ../src/signalhandler.h   \
    ../src/welcometab.h      \
    ../src/selecttab.h       \
    ../src/preparetab.h      \
    ../src/printtab.h        \
    ../src/statustab.h       \
    ../src/advancedtab.h     \
    ../src/constants.h       \
    ../src/utils.h           \
    ../src/gesturelistview.h \
    ../src/hasher.h

CONFIG += c++17
CONFIG += precompile_header
PRECOMPILED_HEADER = ../src/pch.h

RESOURCES += ../gl/gl.qrc ../breeze/breeze.qrc

linux {
    target.path = /usr/bin
    INSTALLS += target
}

static {
    CONFIG += static
}

debug {
    QMAKE_CXXFLAGS_DEBUG += -D_DEBUG
}

release {
    QMAKE_CXXFLAGS_RELEASE += -DNDEBUG
}

warn_on {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-class-memaccess -Wno-unused-parameter
}
