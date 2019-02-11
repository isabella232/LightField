QT += core gui opengl widgets xml

TARGET = fstl
TEMPLATE = app

# Bump optimization up to -O3 in release builds
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES += \
    ../src/app.cpp\
    ../src/main.cpp\
    ../src/canvas.cpp \
    ../src/mesh.cpp \
    ../src/glmesh.cpp \
    ../src/loader.cpp \
    ../src/window.cpp \
    ../src/backdrop.cpp \
    ../src/shepherd.cpp \
    ../src/slicer.cpp \
    ../src/strings.cpp \
    ../src/printmanager.cpp \
    ../src/pngdisplayer.cpp \
    ../src/svgrenderer.cpp \
    ../src/processrunner.cpp \
    ../src/signalhandler.cpp \
    ../src/selecttab.cpp \
    ../src/preparetab.cpp \
    ../src/printtab.cpp \
    ../src/statustab.cpp

HEADERS  += \
    ../src/app.h\
    ../src/canvas.h \
    ../src/mesh.h \
    ../src/glmesh.h \
    ../src/loader.h \
    ../src/window.h \
    ../src/backdrop.h \
    ../src/shepherd.h \
    ../src/slicer.h \
    ../src/strings.h \
    ../src/printmanager.h \
    ../src/printjob.h \
    ../src/pngdisplayer.h \
    ../src/svgrenderer.h \
    ../src/processrunner.h \
    ../src/signalhandler.h \
    ../src/debug.h \
    ../src/selecttab.h \
    ../src/preparetab.h \
    ../src/printtab.h \
    ../src/statustab.h

CONFIG += c++11
CONFIG += precompile_header
PRECOMPILED_HEADER = ../src/pch.h

RESOURCES += \
    qt.qrc \
    ../gl/gl.qrc

macx {
    QMAKE_INFO_PLIST = ../app/Info.plist
    ICON = ../app/fstl.icns
}

win32 {
    RC_FILE = ../exe/fstl.rc
}

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
    QMAKE_CXXFLAGS_WARN_ON += -Wno-class-memaccess
}
