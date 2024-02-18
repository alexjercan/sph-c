cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_COMPILER "/usr/bin/clang")

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    ini_h
    GIT_REPOSITORY "https://github.com/alexjercan/ini.h.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(ini_h)

project(sphlib C)

file(GLOB_RECURSE SPH_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.c")
add_library(sphlib STATIC ${SPH_SOURCES})
target_include_directories(sphlib PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(sphlib PRIVATE raylib)

file(GLOB EXAMPLE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/examples/*.c")
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE ${PROJECT_NAME})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE raylib)
endforeach()

project(main C)

add_executable(main "${CMAKE_CURRENT_LIST_DIR}/main.c")
target_include_directories(main PRIVATE ${CMAKE_BINARY_DIR}/_deps/ini_h-src)
target_link_libraries(main PRIVATE sphlib)
target_link_libraries(main PRIVATE raylib)
