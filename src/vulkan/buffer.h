#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include <array>
#include "../core/core.h"

struct Vertex 
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 tex_coord;
  glm::vec3 normal;

  static VkVertexInputBindingDescription get_binding_description()
  {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
  }

    static std::array<VkVertexInputAttributeDescription, 4> get_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};

        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

        attribute_descriptions[3].binding = 0;
        attribute_descriptions[3].location = 3;
        attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[3].offset = offsetof(Vertex, normal);

        return attribute_descriptions;
    }
};

// VulkanBuffer is a stateless-ish helper: it owns no vertex/index data and no
// GPU buffers itself. It just knows how to allocate+bind a VkBuffer against
// this device. Callers (like Mesh) own their own VkBuffer/VkDeviceMemory
// handles and lifetime, and call create_buffer() to fill them in.
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
