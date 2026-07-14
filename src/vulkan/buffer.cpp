#include "buffer.h"
#include "context.h"
#include <vector>
#include <vulkan/vulkan_core.h>

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // top-left
    {{ 0.5f, -0.5f, 0}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // top-right
    {{ 0.5f,  0.5f, 0}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // bottom-right
    {{-0.5f,  0.5f, 0}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}  // bottom-left
};

const std::vector<uint16_t> indices = {
    0, 1, 2,
    2, 3, 0
};

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

void VulkanBuffer::create_vertex_buffer()
{
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = sizeof(vertices[0]) * vertices.size();
  buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_context.get_device(), &buffer_info, nullptr,
        &m_vertex_buffer) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to create vertex buffer");
  }
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_context.get_device(), m_vertex_buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(m_context.get_device(), &allocInfo,
        nullptr, &m_vertex_buffer_memory) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(m_context.get_device(), m_vertex_buffer, m_vertex_buffer_memory, 0);

  void* data;
  vkMapMemory(m_context.get_device(), m_vertex_buffer_memory, 0, buffer_info.size, 0, &data);
  memcpy(data, vertices.data(), (size_t) buffer_info.size);
  vkUnmapMemory(m_context.get_device(), m_vertex_buffer_memory);

  m_vertex_count = static_cast<uint32_t>(vertices.size());

  std::print(GREEN "Pass:  " RESET "Create Buffer\n");
}

void VulkanBuffer::create_index_buffer()
{
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = sizeof(indices[0]) * indices.size();
  buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_context.get_device(), &buffer_info, nullptr,
        &m_index_buffer) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to create index buffer");
  }
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_context.get_device(), m_index_buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(m_context.get_device(), &allocInfo,
        nullptr, &m_index_buffer_memory) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to allocate index buffer memory!");
  }

  vkBindBufferMemory(m_context.get_device(), m_index_buffer, m_index_buffer_memory, 0);

  void* data;
  vkMapMemory(m_context.get_device(), m_index_buffer_memory, 0, buffer_info.size, 0, &data);
  memcpy(data, indices.data(), (size_t) buffer_info.size);
  vkUnmapMemory(m_context.get_device(), m_index_buffer_memory);

  m_index_count = static_cast<uint32_t>(indices.size());

  std::print(GREEN "Pass:  " RESET "Create Index Buffer\n");
}

VulkanBuffer::VulkanBuffer(VulkanContext &context) : m_context(context)
{
  create_vertex_buffer();
  create_index_buffer();
}
VulkanBuffer::~VulkanBuffer()
{
  if (m_vertex_buffer != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(m_context.get_device(), m_vertex_buffer, nullptr);
  }
  if (m_vertex_buffer_memory != VK_NULL_HANDLE)
  {
    vkFreeMemory(m_context.get_device(), m_vertex_buffer_memory, nullptr);
  }
  if (m_index_buffer != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(m_context.get_device(), m_index_buffer, nullptr);
  }
  if (m_index_buffer_memory != VK_NULL_HANDLE)
  {
    vkFreeMemory(m_context.get_device(), m_index_buffer_memory, nullptr);
  }
}
