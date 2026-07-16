#include "buffer.h"
#include "context.h"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

VulkanBuffer::VulkanBuffer(VulkanContext &context) : m_context(context)
{
}

uint32_t VulkanBuffer::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_context.get_physical_device(), &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanBuffer::create_buffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &buffer,
    VkDeviceMemory &buffer_memory)
{
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_context.get_device(), &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error(RED "failed to create buffer" RESET);
  }

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(m_context.get_device(), buffer, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_context.get_device(), &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
    throw std::runtime_error(RED "failed to allocate buffer memory!" RESET);
  }

  vkBindBufferMemory(m_context.get_device(), buffer, buffer_memory, 0);
}
