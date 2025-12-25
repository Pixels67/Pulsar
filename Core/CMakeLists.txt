project(PulsarCore)

find_package(Vulkan REQUIRED)

CPMAddPackage(
        URI "gh:glfw/glfw#3.4"
        OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
)

add_library(${PROJECT_NAME}
        include/Pulsar.hpp
        src/Pulsar.cpp
        src/Vulkan/Instance.cpp
        src/Vulkan/Instance.hpp
        src/Version.hpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include src)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan glfw)