#include "mesh.h"
#include "command.h"
#include "src/vulkan/camera.h"
#include "src/vulkan/swapchain.h"
#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan_core.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void Mesh::load(std::string model_path, std::string texture_path)
{
  cgltf_options options = {};
  cgltf_data* data = NULL;

  cgltf_result result = cgltf_parse_file(&options, model_path.c_str(), &data);
  if (result != cgltf_result_success) {
    throw std::runtime_error("failed to parse glb: " + model_path);
  }

  result = cgltf_load_buffers(&options, data, model_path.c_str());
  if (result != cgltf_result_success) {
    cgltf_free(data);
    throw std::runtime_error("failed to load glb buffers: " + model_path);
  }

  for (size_t i = 0; i < data->meshes_count; i++) {
    cgltf_mesh* mesh = &data->meshes[i];

    for (size_t p = 0; p < mesh->primitives_count; p++) {
      cgltf_primitive* prim = &mesh->primitives[p];

      cgltf_accessor* pos_accessor    = nullptr;
      cgltf_accessor* uv_accessor     = nullptr;
      cgltf_accessor* color_accessor  = nullptr;
      cgltf_accessor* normal_accessor = nullptr;

      for (size_t a = 0; a < prim->attributes_count; a++) {
        cgltf_attribute* attr = &prim->attributes[a];

        if (attr->type == cgltf_attribute_type_position && attr->index == 0)
          pos_accessor = attr->data;
        else if (attr->type == cgltf_attribute_type_texcoord && attr->index == 0)
          uv_accessor = attr->data;
        else if (attr->type == cgltf_attribute_type_color && attr->index == 0)
          color_accessor = attr->data;
        else if (attr->type == cgltf_attribute_type_normal && attr->index == 0)
          normal_accessor = attr->data;
      }

      if (!pos_accessor) continue; 

      size_t vertex_offset = m_vertex.size();

      for (size_t v = 0; v < pos_accessor->count; v++) {
        Vertex vert{};

        cgltf_accessor_read_float(pos_accessor, v, &vert.pos.x, 3);

        if (uv_accessor)
          cgltf_accessor_read_float(uv_accessor, v, &vert.tex_coord.x, 2);
        else
          vert.tex_coord = {0.0f, 0.0f};

        if (color_accessor)
          cgltf_accessor_read_float(color_accessor, v, &vert.color.x, 3);
        else
          vert.color = {1.0f, 1.0f, 1.0f};

        if (normal_accessor)
          cgltf_accessor_read_float(normal_accessor, v, &vert.normal.x, 3);
        else
          vert.normal = {1.0f, 1.0f, 1.0f};

        m_vertex.push_back(vert);
      }

      cgltf_accessor* idx_accessor = prim->indices;
      for (size_t k = 0; k < idx_accessor->count; k++) {
        cgltf_uint idx;
        cgltf_accessor_read_uint(idx_accessor, k, &idx, 1);
        m_indices.push_back(static_cast<uint32_t>(idx + vertex_offset));
      }
    }
  }

  cgltf_free(data);
}

Mesh::Mesh(VulkanContext& context, VulkanSwapchain &swapchain, Descriptor& descriptor, VulkanBuffer& buffer) 
    : m_context(context), 
      m_buffer(buffer),
      m_descriptor(descriptor),
      m_texture(context, swapchain, buffer),
      m_material(context, descriptor, m_texture)
{
    load("assets/shapes/capsule_low.glb", "assets/textures/green.png");
    create_buffers();
}

Mesh::~Mesh()
{
  if (m_vertex_buffer != VK_NULL_HANDLE)
    vkDestroyBuffer(m_context.get_device(), m_vertex_buffer, nullptr);
  if (m_vertex_buffer_memory != VK_NULL_HANDLE)
    vkFreeMemory(m_context.get_device(), m_vertex_buffer_memory, nullptr);

  if (m_index_buffer != VK_NULL_HANDLE)
    vkDestroyBuffer(m_context.get_device(), m_index_buffer, nullptr);
  if (m_index_buffer_memory != VK_NULL_HANDLE)
    vkFreeMemory(m_context.get_device(), m_index_buffer_memory, nullptr);
}

void Mesh::update(uint32_t current_frame)
{
    void* data = m_descriptor.get_uniform_mapped(current_frame);
    if (!data) {
        throw std::runtime_error("Failed to get mapped uniform buffer memory");
    }

    MESH_UBO mesh_ubo{};
    mesh_ubo.model = glm::mat4(1.0f); 
    memcpy(static_cast<char*>(data) + sizeof(CAMERA_UBO), &mesh_ubo, sizeof(MESH_UBO));
}

void Mesh::record(
    VulkanCommands& commands,
    uint32_t frame_index,
    VkRenderPass render_pass,
    VkFramebuffer framebuffer,
    VkPipeline pipeline,
    VkPipelineLayout pipeline_layout,
    VkDescriptorSet descriptor_set,
    VkExtent2D extent)
{
  commands.record_command_buffer(
      frame_index, render_pass, framebuffer, pipeline, pipeline_layout,
      descriptor_set, extent,
      m_vertex_buffer, get_vertex_count(),
      m_index_buffer, get_index_count(),
      VK_INDEX_TYPE_UINT32
  );
}

void Mesh::create_buffers()
{
    VkDevice device = m_context.get_device();

    // --- Vertex buffer ---
    VkDeviceSize vertex_buffer_size = sizeof(Vertex) * m_vertex.size();

    m_buffer.create_buffer(
        vertex_buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_vertex_buffer,
        m_vertex_buffer_memory);

    void* data;
    vkMapMemory(device, m_vertex_buffer_memory, 0, vertex_buffer_size, 0, &data);
    memcpy(data, m_vertex.data(), (size_t)vertex_buffer_size);
    vkUnmapMemory(device, m_vertex_buffer_memory);

    // --- Index buffer ---
    VkDeviceSize index_buffer_size = sizeof(uint32_t) * m_indices.size();

    m_buffer.create_buffer(
        index_buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_index_buffer,
        m_index_buffer_memory);

    vkMapMemory(device, m_index_buffer_memory, 0, index_buffer_size, 0, &data);
    memcpy(data, m_indices.data(), (size_t)index_buffer_size);
    vkUnmapMemory(device, m_index_buffer_memory);
}
