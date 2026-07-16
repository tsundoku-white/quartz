#pragma once

// STL - frequently used
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <print>
#include <exception>
#include <stdexcept>

// Vulkan - used everywhere
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// GLFW - used everywhere
#define INCLUDE_GLFW_VULKAN
#include <GLFW/glfw3.h>

// GLM - used everywhere
#include <glm/glm.hpp>

// Project macros
#define RED "\e[0;31m"
#define YELLOW "\e[0;33m"
#define GREEN "\e[0;32m"
#define RESET "\e[0m"

// Common forward declarations
class Window;
class VulkanContext;
class VulkanSwapchain;
class VulkanRenderer;
class VulkanCommands;
class VulkanBuffer;
class VulkanSync;
class Camera;
class Texture;
class Depth;
