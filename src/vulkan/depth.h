#pragma once

#include "../core/core.h"
#include "context.h"
#include "src/vulkan/swapchain.h"

class Depth
{
  public:
    Depth(VulkanContext& context, VulkanSwapchain &swapchain);
    ~Depth();

    Depth(const Depth&) = delete;
    Depth& operator=(const Depth&) = delete;

    void create_depth_resources(VkExtent2D extent);
    void cleanup();

    [[nodiscard]] VkImageView get_image_view() const { return m_depth_image_view; }
    [[nodiscard]] VkFormat    get_format()     const { return m_depth_format;     }

  private:
    VulkanContext& m_context;
    VulkanSwapchain &m_swapchain;

    VkImage        m_depth_image        = VK_NULL_HANDLE;
    VkDeviceMemory m_depth_image_memory = VK_NULL_HANDLE;
    VkImageView    m_depth_image_view   = VK_NULL_HANDLE;
    VkFormat       m_depth_format       = VK_FORMAT_UNDEFINED;

    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat find_depth_format();
    bool     has_stencil_component(VkFormat format);

    void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                       VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                       VkImage& image, VkDeviceMemory& image_memory);
    VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
};
