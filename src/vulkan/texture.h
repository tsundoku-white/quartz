#pragma once

#include "src/vulkan/swapchain.h"
#include "stb_image.h"

#include "../core/core.h"
#include <vulkan/vulkan_core.h>

class Texture
{
  public:
    Texture(VulkanContext &context, VulkanSwapchain &swapchain, VulkanBuffer &buffer);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    VkSampler   get_sampler()    const { return m_texture_sampler; }
    VkImageView get_image_view() const { return m_texture_image_view; }
  private:
    VulkanContext   &m_context;
    VulkanBuffer    &m_buffer;
    VulkanSwapchain &m_swapchain;

    VkBuffer       m_staging_buffer         = VK_NULL_HANDLE;
    VkDeviceMemory m_staging_buffer_memory  = VK_NULL_HANDLE;
    VkImage        m_texture_image          = VK_NULL_HANDLE;
    VkDeviceMemory m_texture_image_memory   = VK_NULL_HANDLE;
    VkCommandPool  m_command_pool           = VK_NULL_HANDLE;
    VkImageView    m_texture_image_view     = VK_NULL_HANDLE;
    VkSampler      m_texture_sampler        = VK_NULL_HANDLE;

    uint32_t m_tex_width  = 0;
    uint32_t m_tex_height = 0;
    std::string m_path = "assets/textures/default_texture.jpg";

    void create_command_pool();
    void create_texture_image();
    void create_texture_image_view();
    void create_texture_sampler();

    VkImageView create_image_view(VkImage image, VkFormat format);

    void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                       VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                       VkImage& image, VkDeviceMemory& image_memory);

    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer command_buffer);

    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};
