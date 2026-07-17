#pragma once
#include "../core/core.h"
#include "texture.h"

struct MaterialHandle
{
    uint32_t index = UINT32_MAX;
    [[nodiscard]] bool is_valid() const { return index != UINT32_MAX; }
};

struct MaterialPool
{
    std::vector<std::vector<VkDescriptorSet>> descriptor_sets;
    std::vector<TextureHandle> texture;

    uint32_t count = 0;
};

namespace material_system
{
  MaterialHandle create(MaterialPool& pool, VulkanContext& context, Descriptor& descriptor,
                                        TexturePool& texture_pool, TextureHandle texture_handle,
                                        const std::vector<VkBuffer>& mesh_ubo_buffers);

    void destroy(MaterialPool& pool, MaterialHandle handle);

    [[nodiscard]] inline VkDescriptorSet get_descriptor_set(const MaterialPool& pool, MaterialHandle handle, uint32_t frame)
    {
        return pool.descriptor_sets[handle.index][frame];
    }
}
