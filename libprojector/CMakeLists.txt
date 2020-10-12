cmake_minimum_required(VERSION 3.13.4)

project(projector VERSION 1.0.0)
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

add_library(projector SHARED
    src/libprojector.cpp
    src/dlp4710impl.cpp
    src/dlpc350impl.cpp
)

add_executable(setprojectorpower 
    src/setprojectorpower.cpp
)

target_link_libraries(setprojectorpower projector hidapi-libusb)
