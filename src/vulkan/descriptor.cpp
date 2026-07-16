#include "descriptor.h"
#include "context.h"
#include "buffer.h"
#include "swapchain.h"
#include "sync.h"
#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "camera.h"
#include "mesh.h"
#include "light.h"

Descriptor::Descriptor(VulkanContext &context, VulkanSwapchain &swapchain, VulkanSync &sync, VulkanBuffer &buffer)
    : m_context(context), m_swapchain(swapchain), m_sync(sync), m_buffer(buffer)
{
    create_descriptor_layout();
    create_uniform_buffers();
    create_descriptor_pool();
}

Descriptor::~Descriptor()
{
    if (m_descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(m_context.get_device(), m_descriptor_pool, nullptr);

    for (size_t i = 0; i < m_uniform_buffers.size(); i++)
    {
        vkDestroyBuffer(m_context.get_device(), m_uniform_buffers[i], nullptr);
        vkFreeMemory(m_context.get_device(), m_uniform_buffers_memory[i], nullptr);
    }

    if (m_descriptor_set_layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(m_context.get_device(), m_descriptor_set_layout, nullptr);
}

void Descriptor::create_descriptor_layout()
{
  VkDescriptorSetLayoutBinding camera_ubo_binding{};
  camera_ubo_binding.binding = 0;
  camera_ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  camera_ubo_binding.descriptorCount = 1;
  camera_ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  camera_ubo_binding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutBinding mesh_ubo_binding{};
  mesh_ubo_binding.binding = 1;
  mesh_ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  mesh_ubo_binding.descriptorCount = 1;
  mesh_ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  mesh_ubo_binding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutBinding sun_ubo_binding{};
  sun_ubo_binding.binding = 3;
  sun_ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sun_ubo_binding.descriptorCount = 1;
  sun_ubo_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; 

  VkDescriptorSetLayoutBinding sampler_layout_binding{};
  sampler_layout_binding.binding = 2;
  sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.descriptorCount = 1;
  sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  sampler_layout_binding.pImmutableSamplers = nullptr;

  std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
    camera_ubo_binding, mesh_ubo_binding, sun_ubo_binding, sampler_layout_binding
  };

  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_info.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(m_context.get_device(), &layout_info,
        nullptr, &m_descriptor_set_layout) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to create descriptor set layout!" RESET);
  }
}

void Descriptor::create_uniform_buffers()
{
  // Calculate total size needed for both UBOs
  VkDeviceSize camera_buffer_size = sizeof(CAMERA_UBO);
  VkDeviceSize mesh_buffer_size   = sizeof(MESH_UBO);
  VkDeviceSize sun_buffer_size    = sizeof(SUN_UBO);
  VkDeviceSize total_buffer_size  = camera_buffer_size + mesh_buffer_size + sun_buffer_size;

  m_uniform_buffers.resize(m_sync.get_frame_count());
  m_uniform_buffers_memory.resize(m_sync.get_frame_count());
  m_uniform_buffers_mapped.resize(m_sync.get_frame_count());

  for (size_t i = 0; i < m_sync.get_frame_count(); i++)
  {
    m_buffer.create_buffer(
        total_buffer_size, 
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_uniform_buffers[i],
        m_uniform_buffers_memory[i]);

    vkMapMemory(m_context.get_device(), m_uniform_buffers_memory[i], 0, total_buffer_size, 0, &m_uniform_buffers_mapped[i]);
  }

  std::print(GREEN "Pass:  " RESET "Create uniform buffers\n");
}

void Descriptor::create_descriptor_pool()
{
  constexpr uint32_t kMaxMaterials = 16;

  std::array<VkDescriptorPoolSize, 2> pool_sizes{};
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_sizes[0].descriptorCount = static_cast<uint32_t>(m_sync.get_frame_count()) * kMaxMaterials * 3; 
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = static_cast<uint32_t>(m_sync.get_frame_count()) * kMaxMaterials;

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.maxSets = static_cast<uint32_t>(m_sync.get_frame_count()) * kMaxMaterials;

  if (vkCreateDescriptorPool(m_context.get_device(), &pool_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to create descriptor pool!" RESET);
  }
}
