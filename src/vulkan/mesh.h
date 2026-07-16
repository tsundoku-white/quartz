#pragma once

#include "material.h"
#include "buffer.h"
#include "command.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/texture.h"
#include <cstdint>
#include <strings.h>
#include <vector>
#include <vulkan/vulkan_core.h>

struct alignas(16) MESH_UBO
{
  glm::mat4 model;
};

class Mesh
{
public:
    Mesh(VulkanContext& context, VulkanSwapchain &swapchain, Descriptor& descriptor, VulkanBuffer& buffer);
    ~Mesh();

    void load(std::string model_path = "assets/shapes/capsule_low.glb",
              std::string texture_path = "assets/textures/default_texture.jpg");

    void update(uint32_t current_frame);

    void record(
        VulkanCommands& commands,
        uint32_t frame_index,
        VkRenderPass render_pass,
        VkFramebuffer framebuffer,
        VkPipeline pipeline,
        VkPipelineLayout pipeline_layout,
        VkDescriptorSet descriptor_set,
        VkExtent2D extent
    );

    [[nodiscard]] VkBuffer get_vertex_buffer() const { return m_vertex_buffer; }
    [[nodiscard]] VkBuffer get_index_buffer()  const { return m_index_buffer; }
    [[nodiscard]] uint32_t get_vertex_count()  const { return static_cast<uint32_t>(m_vertex.size()); }
    [[nodiscard]] uint32_t get_index_count()   const { return static_cast<uint32_t>(m_indices.size()); }

    Material& get_material() { return m_material; }
    Texture& get_texture() { return m_texture; }

private:
    VulkanContext& m_context;
    VulkanBuffer& m_buffer;
    Descriptor& m_descriptor;
    
    Texture  m_texture;  
    Material m_material;   
    
    std::vector<Vertex> m_vertex;
    std::vector<uint32_t> m_indices;

    VkBuffer       m_vertex_buffer        = VK_NULL_HANDLE;
    VkDeviceMemory m_vertex_buffer_memory = VK_NULL_HANDLE;

    VkBuffer       m_index_buffer         = VK_NULL_HANDLE;
    VkDeviceMemory m_index_buffer_memory  = VK_NULL_HANDLE;

    void create_buffers();

};
