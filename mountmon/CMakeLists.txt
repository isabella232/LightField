cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS ON)
set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

set(MOUNTMON_VERSION_MAJOR "1")
set(MOUNTMON_VERSION_MINOR "0")
set(MOUNTMON_VERSION_PATCH "0")
set(PROJECT_VERSION "${MOUNTMON_VERSION_MAJOR}.${MOUNTMON_VERSION_MINOR}.${MOUNTMON_VERSION_PATCH}")

if(WIN32)
set (Qt5_DIR "F:\\\\Qt\\\\5.13.1\\\\msvc2017_64\\\\lib\\\\cmake\\\\Qt5")
endif(WIN32)

project(mountmon)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC OFF)

set(Project_Sources src/main.cpp
src/commandreader.cpp
src/processrunner.cpp
src/signalhandler.cpp
src/udisks.cpp
src/udisksmonitor.cpp
src/usbdevicemounter.cpp)

set(Project_Headers src/commandreader.h
src/processrunner.h
src/signalhandler.h
src/udisks.h
src/udisksmonitor.h
src/usbdevicemounter.h)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Threads REQUIRED)

add_executable(mountmon WIN32 ${Project_Sources} ${Project_Headers})
target_link_libraries(mountmon Qt5::Core ${CMAKE_THREAD_LIBS_INIT})

target_compile_definitions(mountmon PRIVATE -DMOUNTMON_VERSION="${PROJECT_VERSION}")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MountMon")
set(CPACK_PACKAGE_VERSION_MAJOR ${MOUNTMON_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MOUNTMON_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MOUNTMON_VERSION_PATCH})

install(TARGETS mountmon RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

include(CPack)
