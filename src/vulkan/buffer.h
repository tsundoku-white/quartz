#pragma once
#include "../core/core.h"

class VulkanBuffer 
{
  public:
    VulkanBuffer(VulkanContext &context);
    ~VulkanBuffer() = default;

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties, VkBuffer &buffer,
                        VkDeviceMemory &buffer_memory);

  private:
    VulkanContext &m_context;
};
