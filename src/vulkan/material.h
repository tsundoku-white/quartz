#pragma once
#include <vulkan/vulkan.h>
#include "descriptor.h"
#include "texture.h"
#include "../core/core.h"
#include <vector>

class Material
{
  public:
    Material(VulkanContext& context, Descriptor& descriptor, Texture& texture);
    ~Material();

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    [[nodiscard]] VkDescriptorSet get_descriptor_set(uint32_t frame) const { return m_descriptor_sets[frame]; }

  private:
    VulkanContext& m_context;
    std::vector<VkDescriptorSet> m_descriptor_sets;
};
