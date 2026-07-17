#pragma once
#include "../core/core.h"
#include "texture.h"
#include "material.h"
#include <cstdint>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>
#include "transform.h"

struct MeshHandle
{
    uint32_t index = UINT32_MAX;
    [[nodiscard]] bool is_valid() const { return index != UINT32_MAX; }
};

struct MeshPool
{
    std::vector<std::vector<Vertex>>   vertices;
    std::vector<std::vector<uint32_t>> indices;

    std::vector<VkBuffer>       vertex_buffer;
    std::vector<VkDeviceMemory> vertex_buffer_memory;
    std::vector<VkBuffer>       index_buffer;
    std::vector<VkDeviceMemory> index_buffer_memory;

    std::vector<std::vector<VkBuffer>>       mesh_ubo_buffer;
    std::vector<std::vector<VkDeviceMemory>> mesh_ubo_memory;
    std::vector<std::vector<void*>>          mesh_ubo_mapped;

    std::vector<TransformHandle> transform;

    std::vector<TextureHandle>  texture;
    std::vector<MaterialHandle> material;
    std::vector<std::string>    model_path;

    std::vector<bool>      lod_enabled;

    uint32_t count = 0;

};

namespace mesh_system
{
    MeshHandle create(MeshPool& pool, TransformPool &transforms);

    void load(
        VulkanContext&      context,
        VulkanSwapchain&    swapchain,
        Descriptor&         descriptor,
        VulkanBuffer&       buffer,
        MeshPool&           mesh_pool,
        TexturePool&        texture_pool,
        MaterialPool&       material_pool,
        MeshHandle          handle,
        const std::string&  model_path   = "assets/shapes/cube.glb",
        const std::string&  texture_path = "assets/textures/text.png"
    );

    void destroy(VulkanContext& context, MeshPool& pool, MeshHandle handle);
    void destroy_all(VulkanContext& context, MeshPool& pool);

    void update_all(MeshPool& pool, TransformPool &transforms, uint32_t current_frame);

    void set_position(MeshPool& pool, MeshHandle handle, TransformPool &transforms, glm::vec3 location);
    void set_lod(MeshPool& pool, MeshHandle handle, bool value);

    void record_all(
        VulkanCommands& commands,
        MeshPool& pool,
        uint32_t frame_index,
        VkFramebuffer framebuffer,
        VkPipeline pipeline,
        VkPipelineLayout pipeline_layout,
        VkDescriptorSet descriptor_set,
        VkExtent2D extent);

    [[nodiscard]] inline VkBuffer get_vertex_buffer(const MeshPool& pool, MeshHandle handle)
    {
        return pool.vertex_buffer[handle.index];
    }

    [[nodiscard]] inline VkBuffer get_index_buffer(const MeshPool& pool, MeshHandle handle)
    {
        return pool.index_buffer[handle.index];
    }

    [[nodiscard]] inline uint32_t get_vertex_count(const MeshPool& pool, MeshHandle handle)
    {
        return static_cast<uint32_t>(pool.vertices[handle.index].size());
    }

    [[nodiscard]] inline uint32_t get_index_count(const MeshPool& pool, MeshHandle handle)
    {
        return static_cast<uint32_t>(pool.indices[handle.index].size());
    }

    [[nodiscard]] inline VkDescriptorSet get_material_descriptor(
        const MeshPool& mesh_pool, const MaterialPool& material_pool, MeshHandle handle, uint32_t frame)
    {
        return material_system::get_descriptor_set(material_pool, mesh_pool.material[handle.index], frame);
    }
}
