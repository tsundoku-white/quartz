#include "command.h"
#include <cstdint>
#include <stdexcept>

VulkanCommands::VulkanCommands(VulkanContext& context) 
    : m_context(context)
{
    create_command_pool();
}

VulkanCommands::~VulkanCommands() {
    if (m_command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_context.get_device(), m_command_pool, nullptr);
    }
}

void VulkanCommands::create_command_pool() {
    QueueFamilyIndices queueFamilyIndices = m_context.find_queue_families(m_context.get_physical_device());
    
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queueFamilyIndices.graphics_family.value();
    
    if (vkCreateCommandPool(m_context.get_device(), &pool_info, nullptr, &m_command_pool) != VK_SUCCESS) {
        throw std::runtime_error(RED"Failed to create command pool");
    }
}

void VulkanCommands::create_command_buffers(uint32_t image_count) {
    allocate_command_buffers(image_count);
}

void VulkanCommands::allocate_command_buffers(uint32_t image_count) {
    m_command_buffers.resize(image_count);
    
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = image_count;
    
    if (vkAllocateCommandBuffers(m_context.get_device(), &alloc_info, m_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error(RED "Failed to allocate command buffers");
    }
}

void VulkanCommands::record_command_buffer(
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
    )
{
    
    VkCommandBuffer command_buffer = m_command_buffers[frame_index];
    
    begin_command_buffer(command_buffer);
    
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = extent;
    
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;
    
    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkBuffer vertex_buffers[] = { vertex_buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    end_command_buffer(command_buffer);
}

void VulkanCommands::begin_command_buffer(VkCommandBuffer buffer) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;
    
    if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin command buffer");
    }
}

void VulkanCommands::end_command_buffer(VkCommandBuffer buffer) {
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to end command buffer");
    }
}
