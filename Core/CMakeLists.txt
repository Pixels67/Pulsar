project(PulsarCore)

find_package(Vulkan REQUIRED)

CPMAddPackage(
        URI "gh:glfw/glfw#3.4"
        OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
)

CPMAddPackage("gh:KhronosGroup/SPIRV-Headers#vulkan-sdk-1.4.335.0")
CPMAddPackage("gh:KhronosGroup/SPIRV-Tools#vulkan-sdk-1.4.335.0")
CPMAddPackage("gh:KhronosGroup/glslang#vulkan-sdk-1.4.335.0")

CPMAddPackage(
        URI "gh:google/shaderc#v2025.5"
        OPTIONS
        "SHADERC_SKIP_TESTS ON"
        "SHADERC_SKIP_EXAMPLES ON"
        "SHADERC_SKIP_INSTALL ON"
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
        src/Vulkan/Shader.hpp
        src/Vulkan/Shader.cpp
        src/FileIo/File.hpp
        src/FileIo/File.cpp
        src/Vulkan/Surface.cpp
        src/Vulkan/Surface.hpp
        src/Vulkan/Device.cpp
        src/Vulkan/Device.hpp
        src/Vulkan/Common.hpp
        src/Vulkan/SwapChain.cpp
        src/Vulkan/SwapChain.hpp
        src/Vulkan/ImageViews.cpp
        src/Vulkan/ImageViews.hpp
        src/Vulkan/Pipeline.cpp
        src/Vulkan/Pipeline.hpp
        Pch.hpp
)

target_precompile_headers(${PROJECT_NAME} PUBLIC Pch.hpp)
target_include_directories(${PROJECT_NAME} PUBLIC include src)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan shaderc glfw glad)