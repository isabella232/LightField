QT += core gui opengl widgets

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
    ../src/strings.cpp

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
    ../src/strings.h

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
