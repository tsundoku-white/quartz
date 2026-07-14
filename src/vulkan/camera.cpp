#include "camera.h"
#include "context.h"
#include "swapchain.h"
#include "texture.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "sync.h"
#include "buffer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <array>


void Camera::create_descriptor_layout()
{
  VkDescriptorSetLayoutBinding ubo_layout_binding{};
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_layout_binding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutBinding sampler_layout_binding{};
  sampler_layout_binding.binding = 1;
  sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.descriptorCount = 1;
  sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  sampler_layout_binding.pImmutableSamplers = nullptr;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
    ubo_layout_binding, sampler_layout_binding
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

void Camera::create_uniform_buffers()
{
  VkDeviceSize buffer_size = sizeof(UBO);

  m_uniform_buffers.resize(m_sync.get_frame_count());
  m_uniform_buffers_memory.resize(m_sync.get_frame_count());
  m_uniform_buffers_mapped.resize(m_sync.get_frame_count());

  for (size_t i = 0; i < m_sync.get_frame_count(); i++)
  {
    m_buffer.create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_uniform_buffers[i],
        m_uniform_buffers_memory[i]);

    vkMapMemory(m_context.get_device(), m_uniform_buffers_memory[i], 0, buffer_size, 0, &m_uniform_buffers_mapped[i]);
  }

  std::print(GREEN "Pass:  " RESET "Create uniform buffers\n");
}

void Camera::create_descriptor_pool()
{
  std::array<VkDescriptorPoolSize, 2> pool_sizes{};
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_sizes[0].descriptorCount = static_cast<uint32_t>(m_sync.get_frame_count());
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = static_cast<uint32_t>(m_sync.get_frame_count());

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.maxSets = static_cast<uint32_t>(m_sync.get_frame_count());

  if (vkCreateDescriptorPool(m_context.get_device(), &pool_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to create descriptor pool!" RESET);
  }
}

void Camera::create_descriptor_sets()
{
  std::vector<VkDescriptorSetLayout> layouts(m_sync.get_frame_count(), m_descriptor_set_layout);

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = m_descriptor_pool;
  alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  alloc_info.pSetLayouts = layouts.data();

  m_descriptor_sets.resize(layouts.size());
  if (vkAllocateDescriptorSets(m_context.get_device(), &alloc_info, m_descriptor_sets.data()) != VK_SUCCESS)
  {
    throw std::runtime_error(RED "failed to allocate descriptor sets!" RESET);
  }

  for (size_t i = 0; i < layouts.size(); i++)
  {
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = m_uniform_buffers[i];
    buffer_info.offset = 0;
    buffer_info.range = sizeof(UBO);

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView   = m_texture.get_image_view();
    image_info.sampler     = m_texture.get_sampler();

    std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = m_descriptor_sets[i];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &buffer_info;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = m_descriptor_sets[i];
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pImageInfo = &image_info;

    vkUpdateDescriptorSets(m_context.get_device(),
        static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
  }

  std::print(GREEN "Pass:  " RESET "Create descriptor sets\n");
}

void Camera::update(uint32_t current_image, VkExtent2D extent)
{
  UBO ubo{};
  ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 1.0f));

  glm::vec3 forward = get_forward_vector();
  glm::vec3 up = get_up_vector();

  ubo.view = glm::lookAt(m_loc, m_loc + forward, up);

    ubo.proj = glm::perspective(
        glm::radians(m_fov),
        extent.width / (float) extent.height,
        0.1f, 1000.0f
    );

  ubo.proj[1][1] *= -1;
  memcpy(m_uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

void Camera::move(float x, float y, float z, float delta_time)
{
  m_loc += glm::vec3(x, y, z) * m_speed * delta_time;
}

void Camera::move(const glm::vec3& direction, float delta_time)
{
  m_loc += direction * m_speed * delta_time;
}

void Camera::rot(float pitch, float yaw, float roll)
{
  m_rot.x += pitch;
  m_rot.y += yaw;
  m_rot.z += roll;
  
  // Clamp pitch to prevent gimbal lock
  if (m_rot.x > 89.0f) m_rot.x = 89.0f;
  if (m_rot.x < -89.0f) m_rot.x = -89.0f;
  
  // Keep yaw in 0-360 range
  if (m_rot.y > 360.0f) m_rot.y -= 360.0f;
  if (m_rot.y < 0.0f) m_rot.y += 360.0f;
}

glm::vec3 Camera::get_forward_vector() const
{ 
  // Convert degrees to radians
  float pitch = glm::radians(m_rot.x);
  float yaw = glm::radians(m_rot.y);
  
  // Calculate forward vector (looking down -Z by default)
  glm::vec3 forward;
  forward.x = -sin(yaw) * cos(pitch);
  forward.y = -sin(pitch);
  forward.z = -cos(yaw) * cos(pitch);
  
  return glm::normalize(forward);
}

glm::vec3 Camera::get_right_vector() const
{ 
  glm::vec3 forward = get_forward_vector();
  glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  
  // Handle edge case when looking straight up/down
  if (glm::abs(glm::dot(forward, world_up)) > 0.999f) {
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  
  return glm::normalize(glm::cross(forward, world_up));
}

glm::vec3 Camera::get_up_vector() const
{ 
  glm::vec3 forward = get_forward_vector();
  glm::vec3 right = get_right_vector();
  
  return glm::normalize(glm::cross(right, forward));
}

Camera::Camera(VulkanContext &context, VulkanSwapchain &swapchain, VulkanSync &sync, VulkanBuffer &buffer, Texture &texture) 
  : m_context(context), m_swapchain(swapchain), m_sync(sync), m_buffer(buffer), m_texture(texture)
{
  create_descriptor_layout();
  create_uniform_buffers();
  create_descriptor_pool();
  create_descriptor_sets();
  update(0, m_swapchain.get_extent());
}

Camera::~Camera()
{
  if (m_descriptor_pool != VK_NULL_HANDLE)
    vkDestroyDescriptorPool(m_context.get_device(), m_descriptor_pool, nullptr);

  for (size_t i = 0; i < m_sync.get_frame_count(); i++)
  {
    vkDestroyBuffer(m_context.get_device(), m_uniform_buffers[i], nullptr);
    vkFreeMemory(m_context.get_device(), m_uniform_buffers_memory[i], nullptr);
  }

  if (m_descriptor_set_layout != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(m_context.get_device(), m_descriptor_set_layout, nullptr);
}
