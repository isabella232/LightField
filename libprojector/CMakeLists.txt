cmake_minimum_required(VERSION 3.13.4)

project(projector VERSION 1.0.0)
set(CMAKE_BUILD_TYPE Release)

add_library(projector SHARED
    src/libprojector.cpp
)

add_executable(setprojectorpower 
    src/setprojectorpower.cpp
)

target_link_libraries(setprojectorpower projector)
