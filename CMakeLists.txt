cmake_minimum_required(VERSION 3.20)
project(quartz)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(quartz
    src/main.cpp
    src/vulkan/window.cpp
    src/vulkan/context.cpp
    src/vulkan/swapchain.cpp
    src/vulkan/renderer.cpp
    src/vulkan/command.cpp
    src/vulkan/sync.cpp
    src/vulkan/buffer.cpp
    src/vulkan/frame_manager.cpp 
)

target_precompile_headers(quartz PRIVATE
    src/core/core.h
)


# Include directories
target_include_directories(quartz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Vulkan_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(quartz PRIVATE
    glfw
    ${Vulkan_LIBRARIES}
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(quartz PRIVATE
        ${CMAKE_DL_LIBS}
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(quartz PRIVATE
        ${CMAKE_DL_LIBS}
        pthread
        X11
        Xrandr
        Xi
    )
endif()

set(SPIRV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

target_compile_definitions(quartz PUBLIC
    SHADER_PATH="${SPIRV_OUTPUT_DIR}/"
)

find_program(GLSLC glslc)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found")
endif()

file(GLOB_RECURSE GLSL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.glsl"
)

file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

foreach(GLSL ${GLSL_SOURCES})
    get_filename_component(FILE_NAME ${GLSL} NAME_WE)
    set(SPIRV "${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv")

    if(FILE_NAME MATCHES "vert")
        set(STAGE vertex)
    elseif(FILE_NAME MATCHES "frag")
        set(STAGE fragment)
    elseif(FILE_NAME MATCHES "comp")
        set(STAGE compute)
    else()
        message(WARNING "Unknown shader stage for: ${FILE_NAME}.glsl — skipping")
        continue()
    endif()

    add_custom_command(
        OUTPUT  ${SPIRV}
        COMMAND ${GLSLC} -fshader-stage=${STAGE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        COMMENT "Compiling shader: ${FILE_NAME}.glsl"
        USES_TERMINAL
    )
    list(APPEND SPIRV_OUTPUTS ${SPIRV})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(quartz Shaders)
