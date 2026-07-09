#pragma once

#include <glm/glm.hpp>
#include "../core/core.h"

struct Vertex 
{
  glm::vec3 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription get_binding_description()
  {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
  }

  static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(Vertex, pos);

    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(Vertex, color);
    return attribute_descriptions;
  }
};

class Buffer 
{
  public:
    Buffer(VulkanContext &context);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    [[nodiscard]] VkBuffer get_vertex_buffer() const { return m_vertex_buffer; }
    [[nodiscard]] uint32_t get_vertex_count()  const { return m_vertex_count; }

  private:
    VulkanContext &m_context;
    VkBuffer m_vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertex_buffer_memory = VK_NULL_HANDLE;
    uint32_t m_vertex_count = 0;

    void create_vertex_buffer();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
