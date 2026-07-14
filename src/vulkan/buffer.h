#pragma once

#include <glm/glm.hpp>
#include "../core/core.h"

struct Vertex 
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 tex_coord;

  static VkVertexInputBindingDescription get_binding_description()
  {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
  }

    static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};

        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; 
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

        return attribute_descriptions;
    }
};

struct QuadMesh
{
  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;
};

inline QuadMesh make_quad(float x, float y, float w, float h,
                           float u0, float v0, float u1, float v1,
                           glm::vec3 color = {1.0f, 1.0f, 1.0f})
{
  return {
    { {{x,     y,     0.0f}, color, {u0, v0}},
      {{x + w, y,     0.0f}, color, {u1, v0}},
      {{x + w, y + h, 0.0f}, color, {u1, v1}},
      {{x,     y + h, 0.0f}, color, {u0, v1}} },
    { 0, 1, 2, 2, 3, 0 }
  };
}

class VulkanBuffer 
{
  public:
    VulkanBuffer(VulkanContext &context);
    ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    [[nodiscard]] VkBuffer get_vertex_buffer() const { return m_vertex_buffer; }
    [[nodiscard]] uint32_t get_vertex_count()  const { return m_vertex_count; }

    [[nodiscard]] VkBuffer get_index_buffer() const { return m_index_buffer; }
    [[nodiscard]] uint32_t get_index_count()  const { return m_index_count; }

    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties, VkBuffer &buffer,
                        VkDeviceMemory &buffer_memory);

  private:
    VulkanContext &m_context;
    VkBuffer m_vertex_buffer              = VK_NULL_HANDLE;
    VkDeviceMemory m_vertex_buffer_memory = VK_NULL_HANDLE;
    uint32_t m_vertex_count               = 0;

    VkBuffer m_index_buffer               = VK_NULL_HANDLE;
    VkDeviceMemory m_index_buffer_memory  = VK_NULL_HANDLE;
    uint32_t m_index_count                = 0;

    void create_vertex_buffer();
    void create_index_buffer();
};
