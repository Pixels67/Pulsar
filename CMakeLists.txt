cmake_minimum_required(VERSION 4.0)
project(Pulsar)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler / Linker options
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (MSVC)
        add_compile_options(
                /MDd
        )
    else ()
        add_compile_options(
                -Wall
                -Wextra
                -Wpedantic
                -fsanitize=address
                -g
        )

        add_link_options(-fsanitize=address)
    endif ()
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (MSVC)
        add_compile_options(
                /MD
        )
    else ()
        add_compile_options(
                -O3
        )
    endif ()
endif ()

include(cmake/CPM.cmake)

find_package(Vulkan REQUIRED)

CPMAddPackage(
        URI "gh:glfw/glfw#3.4"
        OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
)

add_executable(Pulsar main.cpp)

target_link_libraries(Pulsar PRIVATE Vulkan::Vulkan glfw)