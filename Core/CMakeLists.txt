project(PulsarCore)

find_package(Vulkan REQUIRED)

CPMAddPackage(
        URI "gh:glfw/glfw#3.4"
        OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
)

CPMAddPackage("gh:Pixels67/glad#master")

add_library(${PROJECT_NAME}
        include/Pulsar.hpp
        src/GraphicsApi.hpp
        src/Version.hpp
        src/Vulkan/Extensions.hpp
        src/Vulkan/Instance.hpp
        src/Vulkan/Instance.cpp
        src/Glfw/Window.hpp
        src/Glfw/Window.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include src)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan glfw glad)