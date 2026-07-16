cmake_minimum_required(VERSION 3.20)
project(quartz)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_GENERATOR Ninja)

if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
    message(FATAL_ERROR "This project requires Ninja. Configure with: cmake --preset default")
endif()

# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

find_path(GLM_INCLUDE_DIR glm/glm.hpp
    PATHS ${CMAKE_SOURCE_DIR}/ext/glm
    REQUIRED
)

find_path(STB_INCLUDE_DIR stb_image.h, stb_truetype.h, stb_image_write.h
    PATHS ${CMAKE_SOURCE_DIR}/ext/stb
    REQUIRED
)
message("STB include dir: ${STB_INCLUDE_DIR}")

find_path(CGLTF_INCLUDE_DIR cgltf.h
    PATHS ${CMAKE_SOURCE_DIR}/ext/cgltf
    REQUIRED
)
message("CLGTF include dir: ${CGLTF_INCLUDE_DIR}")

# Add executable
add_executable(quartz
    src/main.cpp
    src/vulkan/window.cpp

    #vulkan includes
    src/vulkan/context.cpp
    src/vulkan/swapchain.cpp
    src/vulkan/renderer.cpp
    src/vulkan/command.cpp
    src/vulkan/sync.cpp
    src/vulkan/buffer.cpp
    src/vulkan/frame_manager.cpp 
    src/vulkan/descriptor.cpp
    src/vulkan/depth.cpp

    # components?
    src/vulkan/camera.cpp
    src/vulkan/texture.cpp
    #src/vulkan/text.cpp
    src/vulkan/material.cpp
    src/vulkan/mesh.cpp
    src/vulkan/light.cpp
)

target_precompile_headers(quartz PRIVATE
    src/core/core.h
)


target_include_directories(quartz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Vulkan_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
    ${CGLTF_INCLUDE_DIR}
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

add_custom_command(TARGET quartz POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/assets
        $<TARGET_FILE_DIR:quartz>/assets
)

add_custom_target(Shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(quartz Shaders)
