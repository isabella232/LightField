QT += core gui widgets xml serialport

TARGET   = lf
TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -DNDEBUG -Wall -Wextra -Winvalid-pch -Wno-unused-result
QMAKE_CXXFLAGS_DEBUG   += -Og -D_DEBUG -Wall -Wextra -Winvalid-pch -Wno-unused-result

QMAKE_CXXFLAGS += -O0
QMAKE_CXXFLAGS -= -O1
QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS -= -O3

SOURCES +=                          \
    ../src/advancedtab.cpp          \
    ../src/advancedtabselectionmodel.cpp \
    ../src/app.cpp                  \
    ../src/backdrop.cpp             \
    ../src/buildinfo.cpp            \
    ../src/canvas.cpp               \
    ../src/constants.cpp            \
    ../src/debug.cpp                \
    ../src/debuglogcopier.cpp       \
    ../src/filecopier.cpp           \
    ../src/filetab.cpp              \
    ../src/gesturelistview.cpp      \
    ../src/glmesh.cpp               \
    ../src/gpgsignaturechecker.cpp  \
    ../src/hasher.cpp               \
    ../src/key.cpp                  \
    ../src/keyboard.cpp             \
    ../src/lightfieldstyle.cpp      \
    ../src/loader.cpp               \
    ../src/main.cpp                 \
    ../src/mesh.cpp                 \
    ../src/ordermanifestmanager.cpp \
    ../src/movementsequencer.cpp    \
    ../src/paramslider.cpp          \
    ../src/pngdisplayer.cpp         \
    ../src/preparetab.cpp           \
    ../src/printjob.cpp             \
    ../src/printmanager.cpp         \
    ../src/printprofile.cpp         \
    ../src/printprofilemanager.cpp  \
    ../src/printtab.cpp             \
    ../src/processrunner.cpp        \
    ../src/profilestab.cpp          \
    ../src/progressdialog.cpp       \
    ../src/shepherd.cpp             \
    ../src/signalhandler.cpp        \
    ../src/slicesorderpopup.cpp	    \
    ../src/slicertask.cpp           \
    ../src/spoiler.cpp              \
    ../src/statustab.cpp            \
    ../src/stdiologger.cpp          \
    ../src/strings.cpp              \
    ../src/svgrenderer.cpp          \
    ../src/systemtab.cpp            \
    ../src/tabbase.cpp              \
    ../src/timinglogger.cpp         \
    ../src/tilingmanager.cpp        \
    ../src/tilingtab.cpp            \
    ../src/upgradekitunpacker.cpp   \
    ../src/upgrademanager.cpp       \
    ../src/upgradeselector.cpp      \
    ../src/usbmountmanager.cpp      \
    ../src/utils.cpp                \
    ../src/window.cpp \
    ../src/thicknesswindow.cpp \
    ../src/firmwarecontroller.cpp

HEADERS  +=                         \
    ../src/advancedtab.h            \
    ../src/advancedtabselectionmodel.h \
    ../src/app.h                    \
    ../src/backdrop.h               \
    ../src/buildinfo.h              \
    ../src/canvas.h                 \
    ../src/constants.h              \
    ../src/coordinate.h             \
    ../src/debug.h                  \
    ../src/debuglogcopier.h         \
    ../src/filecopier.h             \
    ../src/filetab.h                \
    ../src/gesturelistview.h        \
    ../src/glmesh.h                 \
    ../src/gpgsignaturechecker.h    \
    ../src/hasher.h                 \
    ../src/inputdialog.h            \
    ../src/key.h                    \
    ../src/keyboard.h               \
    ../src/initialshoweventmixin.h  \
    ../src/lightfieldstyle.h        \
    ../src/loader.h                 \
    ../src/mesh.h                   \
    ../src/movementsequencer.h      \
    ../src/spoiler.h                \
    ../src/ordermanifestmanager.h   \
    ../src/paramslider.h            \
    ../src/pngdisplayer.h           \
    ../src/preparetab.h             \
    ../src/printjob.h               \
    ../src/printmanager.h           \
    ../src/printprofile.h           \
    ../src/printprofilemanager.h    \
    ../src/printtab.h               \
    ../src/processrunner.h          \
    ../src/profilesjsonparser.h     \
    ../src/profilestab.h            \
    ../src/progressdialog.h         \
    ../src/shepherd.h               \
    ../src/signalhandler.h          \
    ../src/slicesorderpopup.h	    \
    ../src/slicertask.h             \
    ../src/statustab.h              \
    ../src/stdiologger.h            \
    ../src/strings.h                \
    ../src/svgrenderer.h            \
    ../src/systemtab.h              \
    ../src/tabbase.h                \
    ../src/timinglogger.h           \
    ../src/tilingmanager.h          \
    ../src/tilingtab.h              \
    ../src/upgradekitunpacker.h     \
    ../src/upgrademanager.h         \
    ../src/upgradeselector.h        \
    ../src/usbmountmanager.h        \
    ../src/utils.h                  \
    ../src/version.h                \
    ../src/vertex.h                 \
    ../src/window.h \
    ../src/thicknesswindow.h \
    ../src/firmwarecontroller.h

CONFIG += c++1z precompile_header link_pkgconfig
PKGCONFIG = GraphicsMagick++
PRECOMPILED_HEADER = ../src/pch.h

RESOURCES += \
    ../gl/gl.qrc         \
    ../images/images.qrc \
    ../text/text.qrc \
    ../styles.qrc

linux {
    target.path = /usr/bin
    INSTALLS += target
}

static {
    CONFIG += static
}

debug {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-class-memaccess
}

xdlp4710-20um {
    DEFINES += XDLP471020UM
    message(Configuring for XDLP471020UM.)
} else {
    dlp4710 {
        DEFINES += DLP4710
        message(Configuring for DLP4710.)
    }

    !dlp4710 {
        message(Configuring for DLPC350.)
    }
}

experimental {
    DEFINES += EXPERIMENTAL
    message(Configuring for EXPERIMENTAL.)
}

TARGET   = lf
TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -DNDEBUG -Wall -Wextra -Winvalid-pch -Wno-unused-result
QMAKE_CXXFLAGS_DEBUG   += -Og -D_DEBUG -Wall -Wextra -Winvalid-pch -Wno-unused-result

DISTFILES += \
    ../src/stylesheet.qss
