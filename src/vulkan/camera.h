#pragma once
#include "../core/core.h"
#include "transform.h"
#include <glm/ext/matrix_clip_space.hpp>

struct CameraState
{
    VulkanSwapchain* swapchain  = nullptr;
    Descriptor*      descriptor = nullptr;

    TransformHandle transform{};

    float pitch = 0.0f;
    float yaw   = 0.0f;
    float fov   = 90.0f;
    float speed = 15.0f;

    bool perspective_enabled = true;
};

namespace camera_system
{
    CameraState create(VulkanSwapchain& swapchain, Descriptor& descriptor, TransformPool& transforms);

    void update(CameraState& cam, TransformPool& transforms, uint32_t current_image, VkExtent2D extent);

    void move(CameraState& cam, TransformPool& transforms, float x, float y, float z, float delta_time);
    void move(CameraState& cam, TransformPool& transforms, const glm::vec3& direction, float delta_time);

    void rot(CameraState& cam, TransformPool& transforms, float pitch, float yaw, float roll);
    void rotate(CameraState& cam, TransformPool& transforms, const glm::quat& quat);

    [[nodiscard]] glm::vec3 get_forward_vector(const CameraState& cam, const TransformPool& transforms);
    [[nodiscard]] glm::vec3 get_right_vector(const CameraState& cam, const TransformPool& transforms);
    [[nodiscard]] glm::vec3 get_up_vector(const CameraState& cam, const TransformPool& transforms);

    inline void set_perspective(CameraState& cam, bool value) {
        cam.perspective_enabled = value;
    }

    [[nodiscard]] inline glm::mat4 get_view_matrix(const CameraState& cam, const TransformPool& transforms)
    {
        glm::quat rotation = transform_system::get_rotation(transforms, cam.transform);
        glm::vec3 position = transform_system::get_location(transforms, cam.transform);

        glm::mat4 rot_mat   = glm::toMat4(glm::conjugate(rotation));
        glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), -position);
        return rot_mat * trans_mat;
    }

    [[nodiscard]] inline glm::mat4 get_proj_matrix(const CameraState& cam)
    {
        if (cam.perspective_enabled)
        {
            float aspect = 2560.0f / 1440.0f;
            return glm::perspective(glm::radians(cam.fov), aspect, 0.1f, 100.0f);
        }
        else
        {
            return glm::orthoLH(0.0f, 2560.0f, 0.0f, 1440.0f, 0.0f, 1000.0f);
        }
    }
}
