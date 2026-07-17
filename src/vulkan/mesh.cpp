#include "mesh.h"
#include "context.h"
#include "command.h"
#include "descriptor.h"
#include "camera.h"
#include "swapchain.h"
#include "buffer.h"
#include "material.h"
#include "texture.h"
#include <print>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

MeshHandle mesh_system::create(MeshPool& pool)
{
    MeshHandle handle{ pool.count };

    pool.vertices.emplace_back();
    pool.indices.emplace_back();

    pool.vertex_buffer.push_back(VK_NULL_HANDLE);
    pool.vertex_buffer_memory.push_back(VK_NULL_HANDLE);
    pool.index_buffer.push_back(VK_NULL_HANDLE);
    pool.index_buffer_memory.push_back(VK_NULL_HANDLE);

    pool.mesh_ubo_buffer.emplace_back();
    pool.mesh_ubo_memory.emplace_back();
    pool.mesh_ubo_mapped.emplace_back();

    pool.position.push_back({0.0f, 0.0f, 0.0f});
    pool.lod_enabled.push_back(false);

    pool.texture.push_back(TextureHandle{});
    pool.material.push_back(MaterialHandle{});

    pool.count++;
    return handle;
}

void mesh_system::load(
    VulkanContext&      context,
    VulkanSwapchain&    swapchain,
    Descriptor&          descriptor,
    VulkanBuffer&        buffer,
    MeshPool&            mesh_pool,
    TexturePool&         texture_pool,
    MaterialPool&        material_pool,
    MeshHandle           handle,
    const std::string&   model_path,
    const std::string&   texture_path)
{
    cgltf_options options = {};
    cgltf_data* data = nullptr;

    cgltf_result result = cgltf_parse_file(&options, model_path.c_str(), &data);
    if (result != cgltf_result_success) {
        throw std::runtime_error("failed to parse glb: " + model_path);
    }

    result = cgltf_load_buffers(&options, data, model_path.c_str());
    if (result != cgltf_result_success) {
        cgltf_free(data);
        throw std::runtime_error("failed to load glb buffers: " + model_path);
    }

    std::vector<Vertex>&   out_vertices = mesh_pool.vertices[handle.index];
    std::vector<uint32_t>& out_indices  = mesh_pool.indices[handle.index];

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

            size_t vertex_offset = out_vertices.size();

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

                out_vertices.push_back(vert);
            }

            cgltf_accessor* idx_accessor = prim->indices;
            for (size_t k = 0; k < idx_accessor->count; k++) {
                cgltf_uint idx;
                cgltf_accessor_read_uint(idx_accessor, k, &idx, 1);
                out_indices.push_back(static_cast<uint32_t>(idx + vertex_offset));
            }
        }
    }

    cgltf_free(data);

    VkDevice device = context.get_device();

    VkDeviceSize vertex_buffer_size = sizeof(Vertex) * out_vertices.size();
    buffer.create_buffer(
        vertex_buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mesh_pool.vertex_buffer[handle.index],
        mesh_pool.vertex_buffer_memory[handle.index]);

    void* mapped = nullptr;
    vkMapMemory(device, mesh_pool.vertex_buffer_memory[handle.index], 0, vertex_buffer_size, 0, &mapped);
    memcpy(mapped, out_vertices.data(), (size_t)vertex_buffer_size);
    vkUnmapMemory(device, mesh_pool.vertex_buffer_memory[handle.index]);

    VkDeviceSize index_buffer_size = sizeof(uint32_t) * out_indices.size();
    buffer.create_buffer(
        index_buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mesh_pool.index_buffer[handle.index],
        mesh_pool.index_buffer_memory[handle.index]);

    vkMapMemory(device, mesh_pool.index_buffer_memory[handle.index], 0, index_buffer_size, 0, &mapped);
    memcpy(mapped, out_indices.data(), (size_t)index_buffer_size);
    vkUnmapMemory(device, mesh_pool.index_buffer_memory[handle.index]);

    uint32_t frame_count = descriptor.get_frame_count();
    mesh_pool.mesh_ubo_buffer[handle.index].resize(frame_count);
    mesh_pool.mesh_ubo_memory[handle.index].resize(frame_count);
    mesh_pool.mesh_ubo_mapped[handle.index].resize(frame_count);

    for (uint32_t f = 0; f < frame_count; f++) {
        buffer.create_buffer(
            sizeof(MESH_UBO),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mesh_pool.mesh_ubo_buffer[handle.index][f],
            mesh_pool.mesh_ubo_memory[handle.index][f]);

        vkMapMemory(device, mesh_pool.mesh_ubo_memory[handle.index][f], 0, sizeof(MESH_UBO), 0,
                    &mesh_pool.mesh_ubo_mapped[handle.index][f]);
    }

    TextureHandle  texture_handle  = texture_system::create(texture_pool, context, swapchain, buffer, texture_path);
    MaterialHandle material_handle = material_system::create(
        material_pool, context, descriptor, texture_pool, texture_handle,
        mesh_pool.mesh_ubo_buffer[handle.index]);

    mesh_pool.texture[handle.index]  = texture_handle;
    mesh_pool.material[handle.index] = material_handle;
}

void mesh_system::destroy(VulkanContext& context, MeshPool& pool, MeshHandle handle)
{
    VkDevice device = context.get_device();
    uint32_t i = handle.index;

    if (pool.vertex_buffer[i] != VK_NULL_HANDLE)
        vkDestroyBuffer(device, pool.vertex_buffer[i], nullptr);
    if (pool.vertex_buffer_memory[i] != VK_NULL_HANDLE)
        vkFreeMemory(device, pool.vertex_buffer_memory[i], nullptr);

    if (pool.index_buffer[i] != VK_NULL_HANDLE)
        vkDestroyBuffer(device, pool.index_buffer[i], nullptr);
    if (pool.index_buffer_memory[i] != VK_NULL_HANDLE)
        vkFreeMemory(device, pool.index_buffer_memory[i], nullptr);

    for (size_t f = 0; f < pool.mesh_ubo_buffer[i].size(); f++) {
        vkDestroyBuffer(device, pool.mesh_ubo_buffer[i][f], nullptr);
        vkFreeMemory(device, pool.mesh_ubo_memory[i][f], nullptr);
    }

    pool.vertex_buffer[i]        = VK_NULL_HANDLE;
    pool.vertex_buffer_memory[i] = VK_NULL_HANDLE;
    pool.index_buffer[i]         = VK_NULL_HANDLE;
    pool.index_buffer_memory[i]  = VK_NULL_HANDLE;
}

void mesh_system::destroy_all(VulkanContext& context, MeshPool& pool)
{
    for (uint32_t i = 0; i < pool.count; i++) {
        destroy(context, pool, MeshHandle{ i });
    }
}

void mesh_system::update_all(MeshPool& pool, uint32_t current_frame)
{   
    for (uint32_t i = 0; i < pool.count; i++) {
        if (pool.vertex_buffer[i] == VK_NULL_HANDLE) continue;

        MESH_UBO mesh_ubo{};
        mesh_ubo.model = glm::translate(glm::mat4(1.0f), pool.position[i]);
        
        memcpy(pool.mesh_ubo_mapped[i][current_frame], &mesh_ubo, sizeof(MESH_UBO));
    }
}

void mesh_system::set_position(MeshPool& pool, MeshHandle handle, glm::vec3 location)
{
    pool.position[handle.index] = location;
}

void mesh_system::set_lod(MeshPool& pool, MeshHandle handle, bool value)
{
    pool.lod_enabled[handle.index] = value;
}

void mesh_system::record_all(
    VulkanCommands&  commands,
    MeshPool&        pool,
    uint32_t         frame_index,
    VkFramebuffer    framebuffer,
    VkPipeline       pipeline,
    VkPipelineLayout pipeline_layout,
    VkDescriptorSet  descriptor_set,
    VkExtent2D       extent)
{
    for (uint32_t i = 0; i < pool.count; i++) {
        if (pool.vertex_buffer[i] == VK_NULL_HANDLE) continue;

        commands.record_command_buffer(
            frame_index,
            pipeline,
            pipeline_layout,
            descriptor_set, 
            extent,
            pool.vertex_buffer[i],
            static_cast<uint32_t>(pool.vertices[i].size()),
            pool.index_buffer[i],
            static_cast<uint32_t>(pool.indices[i].size()),
            VK_INDEX_TYPE_UINT32
        );
    }
}
