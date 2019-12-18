#### Fast .stl viewer #####
# Original Project Author: Matt Keeter Copyright 2014 -2017
# Author: Paul Tsouchlos Copyright 2017

cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS ON)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

# Set the version number
set (LF_VERSION_MAJOR "1")
set (LF_VERSION_MINOR "0")
set (LF_VERSION_PATCH "0")
set (PROJECT_VERSION "${LF_VERSION_MAJOR}.${LF_VERSION_MINOR}.${LF_VERSION_PATCH}")

if(WIN32)
# Set the Qt5 path
set(Qt5_DIR "F:\\\\Qt\\\\5.14.0\\\\msvc2017_64\\\\lib\\\\cmake\\\\Qt5")
endif(WIN32)

project(lf)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)

#set project sources
set(Project_Sources src/advancedtab.cpp
src/app.cpp
src/backdrop.cpp
src/buildinfo.cpp
src/canvas.cpp
src/constants.cpp
src/debug.cpp
src/debuglogcopier.cpp
src/filecopier.cpp
src/filetab.cpp
src/gesturelistview.cpp
src/glmesh.cpp
src/gpgsignaturechecker.cpp
src/hasher.cpp
src/lightfieldstyle.cpp
src/loader.cpp
src/main.cpp
src/mesh.cpp
src/pngdisplayer.cpp
src/preparetab.cpp
src/printmanager.cpp
src/printtab.cpp
src/processrunner.cpp
src/shepherd.cpp
src/signalhandler.cpp
src/statustab.cpp
src/stdiologger.cpp
src/strings.cpp
src/svgrenderer.cpp
src/systemtab.cpp
src/tabbase.cpp
src/timinglogger.cpp
src/upgradekitunpacker.cpp
src/upgrademanager.cpp
src/upgradeselector.cpp
src/usbmountmanager.cpp
src/utils.cpp
src/window.cpp
)

#set project headers. 
set(Project_Headers src/advancedtab.h
src/app.h
src/backdrop.h
src/buildinfo.h
src/canvas.h
src/constants.h
src/coordinate.h
src/debug.h
src/debuglogcopier.h
src/filecopier.h
src/filetab.h
src/gesturelistview.h
src/glmesh.h
src/gpgsignaturechecker.h
src/hasher.h
src/initialshoweventmixin.h
src/lightfieldstyle.h
src/loader.h
src/mesh.h
src/pngdisplayer.h
src/preparetab.h
src/printjob.h
src/printmanager.h
src/printtab.h
src/processrunner.h
src/shepherd.h
src/signalhandler.h
src/statustab.h
src/stdiologger.h
src/strings.h
src/svgrenderer.h
src/systemtab.h
src/tabbase.h
src/timinglogger.h
src/upgradekitunpacker.h
src/upgrademanager.h
src/upgradeselector.h
src/usbmountmanager.h
src/utils.h
src/version.h
src/vertex.h
src/window.h)

#set project resources and icon resource
set(Project_Resources images/images.qrc gl/gl.qrc text/text.qrc)

#find required packages. 
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml)
find_package(Threads REQUIRED)

#add resources to RCC
qt5_add_resources(Project_Resources_RCC ${Project_Resources})

#tell CMake AUTOGEN to skip autogen on the generated qrc files
set_property(SOURCE ${Project_Resources_RCC} PROPERTY SKIP_AUTOGEN ON)

#include opengl files. 
include_directories(${OPENGL_INCLUDE_DIR} )

add_executable(lf WIN32 ${Project_Sources} ${Project_Headers} ${Project_Resources_RCC} ${Icon_Resource})
target_link_libraries(lf Qt5::Widgets Qt5::Core Qt5::Gui ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
  set(Lf_LINK_FLAGS ${CMAKE_CURRENT_SOURCE_DIR}/${Icon_Resource})
  set_target_properties(lf PROPERTIES LINK_FLAGS ${Lf_LINK_FLAGS})
endif(WIN32)

# Add version definitions to use within the code. 
target_compile_definitions(lf PRIVATE -DLF_VERSION="${PROJECT_VERSION}")

#installer information that is platform independent
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lumen X printer user interface software.")
set(CPACK_PACKAGE_VERSION_MAJOR ${LF_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LF_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LF_VERSION_PATCH})

install(TARGETS lf RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

include(CPack)
