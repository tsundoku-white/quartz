#include "mesh.h"
#include "context.h"
#include "command.h"
#include "descriptor.h"
#include "camera.h"
#include "src/vulkan/transform.h"
#include "swapchain.h"
#include "buffer.h"
#include "material.h"
#include "texture.h"
#include <print>
#include <unordered_map>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

namespace
{
    struct CachedMeshData
    {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
    };
    struct SharedMeshGPU
    {
        VkBuffer       vertex_buffer        = VK_NULL_HANDLE;
        VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;
        VkBuffer       index_buffer         = VK_NULL_HANDLE;
        VkDeviceMemory index_buffer_memory  = VK_NULL_HANDLE;
        int            ref_count            = 0;
    };

    std::unordered_map<std::string, CachedMeshData> g_cpu_mesh_cache;
    std::unordered_map<std::string, SharedMeshGPU>  g_gpu_mesh_cache;
    std::unordered_map<std::string, TextureHandle>  g_texture_cache;

    const CachedMeshData& parse_gltf_cached(const std::string& model_path)
    {
        auto it = g_cpu_mesh_cache.find(model_path);
        if (it != g_cpu_mesh_cache.end())
            return it->second;

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

        CachedMeshData parsed;

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

                size_t vertex_offset = parsed.vertices.size();

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

                    parsed.vertices.push_back(vert);
                }

                cgltf_accessor* idx_accessor = prim->indices;
                for (size_t k = 0; k < idx_accessor->count; k++) {
                    cgltf_uint idx;
                    cgltf_accessor_read_uint(idx_accessor, k, &idx, 1);
                    parsed.indices.push_back(static_cast<uint32_t>(idx + vertex_offset));
                }
            }
        }

        cgltf_free(data);

        auto [inserted_it, ok] = g_cpu_mesh_cache.emplace(model_path, std::move(parsed));
        return inserted_it->second;
    }
}

MeshHandle mesh_system::create(MeshPool& pool, TransformPool &transforms)
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

    pool.transform.push_back(transform_system::create(transforms));
    pool.lod_enabled.push_back(false);

    pool.texture.push_back(TextureHandle{});
    pool.material.push_back(MaterialHandle{});

    pool.model_path.emplace_back();

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
    VkDevice device = context.get_device();
    const CachedMeshData& cached = parse_gltf_cached(model_path);

    std::vector<Vertex>&   out_vertices = mesh_pool.vertices[handle.index];
    std::vector<uint32_t>& out_indices  = mesh_pool.indices[handle.index];
    out_vertices = cached.vertices;
    out_indices  = cached.indices;

    mesh_pool.model_path[handle.index] = model_path;

    auto gpu_it = g_gpu_mesh_cache.find(model_path);
    if (gpu_it == g_gpu_mesh_cache.end()) {
        SharedMeshGPU shared{};

        VkDeviceSize vertex_buffer_size = sizeof(Vertex) * out_vertices.size();
        buffer.create_buffer(
            vertex_buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            shared.vertex_buffer,
            shared.vertex_buffer_memory);

        void* mapped = nullptr;
        vkMapMemory(device, shared.vertex_buffer_memory, 0, vertex_buffer_size, 0, &mapped);
        memcpy(mapped, out_vertices.data(), (size_t)vertex_buffer_size);
        vkUnmapMemory(device, shared.vertex_buffer_memory);

        VkDeviceSize index_buffer_size = sizeof(uint32_t) * out_indices.size();
        buffer.create_buffer(
            index_buffer_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            shared.index_buffer,
            shared.index_buffer_memory);

        vkMapMemory(device, shared.index_buffer_memory, 0, index_buffer_size, 0, &mapped);
        memcpy(mapped, out_indices.data(), (size_t)index_buffer_size);
        vkUnmapMemory(device, shared.index_buffer_memory);

        shared.ref_count = 1;
        gpu_it = g_gpu_mesh_cache.emplace(model_path, shared).first;
    } else {
        gpu_it->second.ref_count++;
    }

    mesh_pool.vertex_buffer[handle.index]        = gpu_it->second.vertex_buffer;
    mesh_pool.vertex_buffer_memory[handle.index] = gpu_it->second.vertex_buffer_memory;
    mesh_pool.index_buffer[handle.index]         = gpu_it->second.index_buffer;
    mesh_pool.index_buffer_memory[handle.index]  = gpu_it->second.index_buffer_memory;

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

    TextureHandle texture_handle;
    auto tex_it = g_texture_cache.find(texture_path);
    if (tex_it == g_texture_cache.end()) {
        texture_handle = texture_system::create(texture_pool, context, swapchain, buffer, texture_path);
        g_texture_cache.emplace(texture_path, texture_handle);
    } else {
        texture_handle = tex_it->second;
    }

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

    const std::string& path = pool.model_path[i];
    auto gpu_it = g_gpu_mesh_cache.find(path);
    if (gpu_it != g_gpu_mesh_cache.end()) {
        gpu_it->second.ref_count--;
        if (gpu_it->second.ref_count <= 0) {
            if (gpu_it->second.vertex_buffer != VK_NULL_HANDLE)
                vkDestroyBuffer(device, gpu_it->second.vertex_buffer, nullptr);
            if (gpu_it->second.vertex_buffer_memory != VK_NULL_HANDLE)
                vkFreeMemory(device, gpu_it->second.vertex_buffer_memory, nullptr);
            if (gpu_it->second.index_buffer != VK_NULL_HANDLE)
                vkDestroyBuffer(device, gpu_it->second.index_buffer, nullptr);
            if (gpu_it->second.index_buffer_memory != VK_NULL_HANDLE)
                vkFreeMemory(device, gpu_it->second.index_buffer_memory, nullptr);

            g_gpu_mesh_cache.erase(gpu_it);
        }
    }

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

void mesh_system::update_all(MeshPool& pool, TransformPool &transforms, uint32_t current_frame)
{
    for (uint32_t i = 0; i < pool.count; i++) {
        if (pool.vertex_buffer[i] == VK_NULL_HANDLE) continue;

        MESH_UBO mesh_ubo{};
        glm::vec3 pos = transform_system::get_location(transforms, pool.transform[i]);
        mesh_ubo.model = glm::translate(glm::mat4(1.0f), pos);

        memcpy(pool.mesh_ubo_mapped[i][current_frame], &mesh_ubo, sizeof(MESH_UBO));
    }
}

void mesh_system::set_position(MeshPool& pool, MeshHandle handle, TransformPool &transforms ,glm::vec3 location)
{
    transform_system::set_location(transforms, pool.transform[handle.index], location);
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
