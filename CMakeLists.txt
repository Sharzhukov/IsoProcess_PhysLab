cmake_minimum_required(VERSION 3.25)

project(IsoProcessLab
    VERSION 0.1.0
    DESCRIPTION "Simulator isoprocess for C++17 & GUI RayLib, for Physics lab."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Автоматическая загрузка Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(isoprocess_lab
    main.cpp
    isoprocess.hpp
    isoprocess.cpp
)

target_link_libraries(isoprocess_lab PRIVATE raylib)

if(APPLE)
    target_link_libraries(isoprocess_lab PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
