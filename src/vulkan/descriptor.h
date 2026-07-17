#pragma once
#include "../core/core.h"

class Descriptor 
{
  public:
    Descriptor(VulkanContext &context, VulkanSwapchain &swapchain, VulkanSync &sync, VulkanBuffer &buffer);
    ~Descriptor();

    Descriptor(const Descriptor&) = delete;
    Descriptor& operator=(const Descriptor&) = delete;

    [[nodiscard]] VkDescriptorSetLayout get_layout() const { return m_descriptor_set_layout; }
    [[nodiscard]] VkDescriptorPool      get_pool()   const { return m_descriptor_pool; }
    [[nodiscard]] VkBuffer get_uniform_buffer(uint32_t frame) const { return m_uniform_buffers[frame]; }
    [[nodiscard]] void*    get_uniform_mapped(uint32_t frame) const { return m_uniform_buffers_mapped[frame]; }
    [[nodiscard]] uint32_t get_frame_count() const { return static_cast<uint32_t>(m_uniform_buffers.size()); }

  private:
    VulkanContext   &m_context;
    VulkanSwapchain &m_swapchain;
    VulkanSync      &m_sync;
    VulkanBuffer    &m_buffer;

    VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorPool      m_descriptor_pool       = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    std::vector<VkBuffer>       m_uniform_buffers;
    std::vector<VkDeviceMemory> m_uniform_buffers_memory;
    std::vector<void*>          m_uniform_buffers_mapped;

    void create_descriptor_layout();
    void create_uniform_buffers();
    void create_descriptor_pool();
};
