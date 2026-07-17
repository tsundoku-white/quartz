#pragma once
#include "../core/core.h"
#include "src/vulkan/context.h"

struct TextureHandle
{
    uint32_t index = UINT32_MAX;
    [[nodiscard]] bool is_valid() const { return index != UINT32_MAX; }
};

struct TexturePool
{
    std::vector<VkBuffer>       staging_buffer;
    std::vector<VkDeviceMemory> staging_buffer_memory;
    std::vector<VkImage>        image;
    std::vector<VkDeviceMemory> image_memory;
    std::vector<VkCommandPool>  command_pool;
    std::vector<VkImageView>    image_view;
    std::vector<VkSampler>      sampler;

    std::vector<uint32_t>    width;
    std::vector<uint32_t>    height;
    std::vector<std::string> path;

    uint32_t count = 0;
};

namespace texture_system
{
    TextureHandle create(TexturePool& pool, VulkanContext& context, VulkanSwapchain& swapchain, VulkanBuffer& buffer,
                          const std::string& path = "assets/textures/green.png");

    void destroy(VulkanContext &conntext, TexturePool& pool, TextureHandle handle);
    void destroy_all(VulkanContext& context, TexturePool& pool);

    [[nodiscard]] inline VkSampler get_sampler(const TexturePool& pool, TextureHandle handle)
    {
        return pool.sampler[handle.index];
    }

    [[nodiscard]] inline VkImageView get_image_view(const TexturePool& pool, TextureHandle handle)
    {
        return pool.image_view[handle.index];
    }
}
