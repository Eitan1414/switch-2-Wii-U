include(/opt/devkitpro/cmake/WiiU.cmake)
cmake_minimum_required(VERSION 3.16)

project(Switch2Mode CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Switch2Mode
    src/main.cpp
    src/config.cpp
    src/intro.cpp
    src/launcher.cpp
    src/wiiu_collections.cpp
    src/ui.cpp
)

wut_create_rpx(Switch2Mode)
wut_create_wuhb(Switch2Mode
    CONTENT content
    NAME "Switch2 Mode"
    SHORTNAME "Switch2 Mode"
    AUTHOR "Eitan1414"
    ICON meta/icon.png
    TVSPLASH meta/tv-splash.png
    DRCSPLASH meta/drc-splash.png
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(sdl2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
pkg_check_modules(SDL2_ttf REQUIRED IMPORTED_TARGET SDL2_ttf)

target_include_directories(Switch2Mode PRIVATE
    src
    /opt/devkitpro/wut/usr/include
    /opt/devkitpro/wums/include
)
target_link_directories(Switch2Mode PRIVATE
    /opt/devkitpro/wut/usr/lib
    /opt/devkitpro/wums/lib
)
target_compile_options(Switch2Mode PRIVATE -Wall -Wextra -Wpedantic -O2)
target_compile_definitions(Switch2Mode PRIVATE SWITCH2MODE_VERSION="0.7.2")

target_link_libraries(Switch2Mode PRIVATE
    mocha
    rpxloader
    PkgConfig::sdl2
    PkgConfig::SDL2_image
    PkgConfig::SDL2_mixer
    PkgConfig::SDL2_ttf
)
