#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include "../core/core.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "descriptor.h"
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

struct alignas(16) CAMERA_UBO 
{
  glm::mat4 view;
  glm::mat4 proj;
};

class Camera
{
  public:
    Camera(VulkanSwapchain &swapchain, Descriptor &descriptor);
    ~Camera();

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

        [[nodiscard]] glm::mat4 get_view_matrix() const {

        glm::mat4 rotation = glm::toMat4(m_quart_rot);
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_loc);
        return rotation * translation;
    }
    
    [[nodiscard]] glm::mat4 get_proj_matrix() const {
        float aspect = 800.0f / 600.0f; 
        return glm::perspective(glm::radians(m_fov), aspect, 0.1f, 100.0f);
    }

    void update(uint32_t current_image, VkExtent2D extent);
    void move(float x, float y, float z, float delta_time);
    void move(const glm::vec3& direction, float delta_time);
    void rot(float pitch, float yaw, float roll);
    void rotate(const glm::quat& quat); 

    glm::vec3 get_forward_vector() const;
    glm::vec3 get_right_vector() const;
    glm::vec3 get_up_vector() const;

    void set_position(const glm::vec3& pos) { m_loc = pos;       }
    void set_rotation(const glm::quat& rot) { m_quart_rot = rot; } 

  private:
    VulkanSwapchain &m_swapchain;
    Descriptor &m_descriptor;

    glm::quat m_quart_rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 m_loc = {0.0f, 0.0f, -5.0f};

    float m_pitch = 0.0f;
    float m_yaw = 0.0f;

    float m_fov = 90.0f;
    float m_speed = 15.0f;
};
