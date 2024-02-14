cmake_minimum_required(VERSION 3.0)
project(sphlib C)
set(CMAKE_C_STANDARD 99)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.c")

# Create a library from your project's source files
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Examples
file(GLOB EXAMPLE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/examples/*.c")
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE ${PROJECT_NAME})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE raylib)
endforeach()

add_executable(main "${CMAKE_CURRENT_LIST_DIR}/main.c")
target_link_libraries(main PRIVATE ${PROJECT_NAME})
target_link_libraries(main PRIVATE raylib)
