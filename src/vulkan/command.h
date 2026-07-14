#pragma once
#include "context.h"
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanCommands {
public:
    VulkanCommands(VulkanContext& context);
    ~VulkanCommands();

    VulkanCommands(const VulkanCommands&) = delete;
    VulkanCommands operator=(const VulkanCommands) = delete;

    void create_command_buffers(uint32_t count);
    void record_command_buffer(
        uint32_t frame_index,
        VkRenderPass render_pass,
        VkFramebuffer framebuffer,
        VkPipeline pipeline,
        VkPipelineLayout pipeline_layout,
        VkDescriptorSet descriptor_set,
        VkExtent2D extent,
        VkBuffer vertex_buffer,
        uint32_t vertex_count,
        VkBuffer index_buffer,
        uint32_t index_count
    );
    
    [[nodiscard]] VkCommandBuffer get_command_buffer(uint32_t index) const { return m_command_buffers[index]; }
    [[nodiscard]] VkCommandPool get_command_pool() const { return m_command_pool; }
    
private:
    VulkanContext& m_context;
    VkCommandPool m_command_pool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_command_buffers;
    
    void create_command_pool();
    void allocate_command_buffers(uint32_t image_count);
    void begin_command_buffer(VkCommandBuffer buffer);
    void end_command_buffer(VkCommandBuffer buffer);
};
