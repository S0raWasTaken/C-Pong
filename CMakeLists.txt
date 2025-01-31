cmake_minimum_required(VERSION 3.10)
project(MyRaylibGame)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/S0ra-default/Desktop/C_Projects/raylib_c/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_PREFIX_PATH "C:/Users/S0ra-default/Desktop/C_Projects/raylib_c/vcpkg/installed/x64-windows" CACHE STRING "vcpkg path")
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

find_package(raylib CONFIG REQUIRED)

add_executable(pong main.c)

target_link_libraries(pong PRIVATE raylib)
