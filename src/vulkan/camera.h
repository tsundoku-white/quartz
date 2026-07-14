#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include "../core/core.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/matrix.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

struct UBO 
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

class Camera
{
  public:
    Camera(VulkanContext &context, VulkanSwapchain &swapchain, VulkanSync &sync, VulkanBuffer &buffer, Texture &texture);
    ~Camera();

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void update(uint32_t current_image, VkExtent2D extent);
    void move(float x, float y, float z, float delta_time);
    void move(const glm::vec3& direction, float delta_time);
    void rot(float pitch, float yaw, float roll);

    [[nodiscard]] VkDescriptorSetLayout get_descriptor_set_layout() const { return m_descriptor_set_layout; }
    [[nodiscard]] VkDescriptorSet       get_descriptor_set(uint32_t frame) const { return m_descriptor_sets[frame]; }

    glm::vec3 get_forward_vector() const;
    glm::vec3 get_right_vector() const;
    glm::vec3 get_up_vector() const;

    void set_position(const glm::vec3& pos) { m_loc = pos; }
    void set_rotation(const glm::vec3& rot) { m_rot = rot; }

  private:
    VulkanContext   &m_context;
    VulkanSwapchain &m_swapchain;
    VulkanSync      &m_sync;
    VulkanBuffer    &m_buffer;
    Texture         &m_texture;

    VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorPool      m_descriptor_pool       = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptor_sets;

    std::vector<VkBuffer>       m_uniform_buffers;
    std::vector<VkDeviceMemory> m_uniform_buffers_memory;
    std::vector<void*>          m_uniform_buffers_mapped;

    glm::vec3 m_loc = {0.0f, 0.0f, -5.0f};
    glm::vec3 m_rot = {0.0f, 180.0f, 0.0f};   
    
    float m_fov = 60.0f;
    float m_speed = 5.0f;

    void create_descriptor_layout();
    void create_uniform_buffers();
    void create_descriptor_pool();
    void create_descriptor_sets();
};
