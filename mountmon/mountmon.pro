QT                     -= gui
QT                     += dbus
CONFIG                 += c++1z cmdline debug precompile_header
win32:CONFIG           += console

TARGET                  = mountmon
TEMPLATE                = app

DESTDIR                 = build
MOC_DIR                 = build
OBJECTS_DIR             = build
VPATH                   = src

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -DNDEBUG -Wextra -Winvalid-pch -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG   += -Og -D_DEBUG -Wextra -Winvalid-pch -Wno-unused-parameter

PRECOMPILED_HEADER      = pch.h

SOURCES                += main.cpp             \
                          signalhandler.cpp    \
                          udisks.cpp           \
                          udisksmonitor.cpp    \
                          usbdevicemounter.cpp

HEADERS                += signalhandler.h      \
                          udisks.h             \
                          udisksmonitor.h      \
                          usbdevicemounter.h
