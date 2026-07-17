#pragma once
#include "../core/core.h"

struct CameraState
{
    VulkanSwapchain* swapchain  = nullptr;
    Descriptor*      descriptor = nullptr;

    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 position = {0.0f, 0.0f, -5.0f};

    float pitch = 0.0f;
    float yaw   = 0.0f;
    float fov   = 90.0f;
    float speed = 15.0f;
};

namespace camera_system
{
    CameraState create(VulkanSwapchain& swapchain, Descriptor& descriptor);

    void update(CameraState& cam, uint32_t current_image, VkExtent2D extent);
    void move(CameraState& cam, float x, float y, float z, float delta_time);
    void move(CameraState& cam, const glm::vec3& direction, float delta_time);
    void rot(CameraState& cam, float pitch, float yaw, float roll);
    void rotate(CameraState& cam, const glm::quat& quat);

    glm::vec3 get_forward_vector(const CameraState& cam);
    glm::vec3 get_right_vector(const CameraState& cam);
    glm::vec3 get_up_vector(const CameraState& cam);

    inline void set_position(CameraState& cam, const glm::vec3& pos) { cam.position = pos; }
    inline void set_rotation(CameraState& cam, const glm::quat& rot) { cam.rotation = rot; }

    [[nodiscard]] inline glm::mat4 get_view_matrix(const CameraState& cam)
    {
        glm::mat4 rotation    = glm::toMat4(cam.rotation);
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), -cam.position);
        return rotation * translation;
    }

    [[nodiscard]] inline glm::mat4 get_proj_matrix(const CameraState& cam)
    {
        float aspect = 800.0f / 600.0f;
        return glm::perspective(glm::radians(cam.fov), aspect, 0.1f, 100.0f);
    }
}
