#include "camera.h"
#include "swapchain.h"
#include "descriptor.h"

CameraState camera_system::create(VulkanSwapchain& swapchain, Descriptor& descriptor, TransformPool& transforms)
{
    CameraState cam{};

    cam.swapchain  = &swapchain;
    cam.descriptor = &descriptor;
    cam.transform  = transform_system::create(transforms);

    transform_system::set_location(transforms, cam.transform, glm::vec3(0.0f, 0.0f, -5.0f));
    transform_system::set_rotation(transforms, cam.transform, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

    update(cam, transforms, 0, swapchain.get_extent());

    return cam;
}

void camera_system::update(CameraState& cam, TransformPool& transforms, uint32_t current_image, VkExtent2D extent)
{
    CAMERA_UBO ubo{};

    ubo.view = get_view_matrix(cam, transforms);

    ubo.proj = glm::perspective(glm::radians(cam.fov),
        extent.width / (float) extent.height, 0.001f, 1000.0f);
    ubo.proj[1][1] *= -1;

    memcpy(cam.descriptor->get_uniform_mapped(current_image), &ubo, sizeof(ubo));
}

void camera_system::move(CameraState& cam, TransformPool& transforms, float x, float y, float z, float delta_time)
{
    glm::vec3 position = transform_system::get_location(transforms, cam.transform);
    position += glm::vec3(x, y, z) * cam.speed * delta_time;
    transform_system::set_location(transforms, cam.transform, position);
}

void camera_system::move(CameraState& cam, TransformPool& transforms, const glm::vec3& direction, float delta_time)
{
    glm::vec3 position = transform_system::get_location(transforms, cam.transform);
    position += direction * cam.speed * delta_time;
    transform_system::set_location(transforms, cam.transform, position);
}

void camera_system::rot(CameraState& cam, TransformPool& transforms, float pitch, float yaw, float roll)
{
    cam.yaw   += yaw;
    cam.pitch += pitch;
    cam.pitch = glm::clamp(cam.pitch, -90.0f, 90.0f);

    glm::quat yaw_quat   = glm::angleAxis(glm::radians(cam.yaw),   glm::vec3(0, 1, 0));
    glm::quat pitch_quat = glm::angleAxis(glm::radians(cam.pitch), glm::vec3(1, 0, 0));

    transform_system::set_rotation(transforms, cam.transform, glm::normalize(yaw_quat * pitch_quat));
}

void camera_system::rotate(CameraState& cam, TransformPool& transforms, const glm::quat& quat)
{
    glm::quat rotation = transform_system::get_rotation(transforms, cam.transform);
    rotation = glm::normalize(quat * rotation);
    transform_system::set_rotation(transforms, cam.transform, rotation);
}

glm::vec3 camera_system::get_forward_vector(const CameraState& cam, const TransformPool& transforms)
{
    glm::quat rotation = transform_system::get_rotation(transforms, cam.transform);
    glm::mat4 rot_mat  = glm::toMat4(rotation);
    glm::vec3 forward  = glm::vec3(rot_mat[2][0], rot_mat[2][1], rot_mat[2][2]);
    return -glm::normalize(forward);
}

glm::vec3 camera_system::get_right_vector(const CameraState& cam, const TransformPool& transforms)
{
    glm::quat rotation = transform_system::get_rotation(transforms, cam.transform);
    glm::mat4 rot_mat  = glm::toMat4(rotation);
    return glm::normalize(glm::vec3(rot_mat[0][0], rot_mat[0][1], rot_mat[0][2]));
}

glm::vec3 camera_system::get_up_vector(const CameraState& cam, const TransformPool& transforms)
{
    glm::quat rotation = transform_system::get_rotation(transforms, cam.transform);
    glm::mat4 rot_mat  = glm::toMat4(rotation);
    return glm::normalize(glm::vec3(rot_mat[1][0], rot_mat[1][1], rot_mat[1][2]));
}
