cmake_minimum_required(VERSION 3.20)
project(quartz)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(VulkanApp 
    src/main.cpp
    src/vulkan/window.cpp
    src/vulkan/context.cpp
    src/vulkan/swapchain.cpp
    src/vulkan/renderer.cpp
    src/vulkan/command.cpp
    src/vulkan/sync.cpp
    src/vulkan/frame_manager.cpp 
)

target_precompile_headers(VulkanApp PRIVATE
    src/core/core.h
)


# Include directories
target_include_directories(VulkanApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Vulkan_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(VulkanApp PRIVATE
    glfw
    ${Vulkan_LIBRARIES}
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(VulkanApp PRIVATE
        ${CMAKE_DL_LIBS}
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(VulkanApp PRIVATE
        ${CMAKE_DL_LIBS}
        pthread
        X11
        Xrandr
        Xi
    )
endif()

# Set output directory
set_target_properties(VulkanApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

