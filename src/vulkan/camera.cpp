#include "camera.h"
#include "swapchain.h"
#include "descriptor.h"

CameraState camera_system::create(VulkanSwapchain& swapchain, Descriptor& descriptor)
{
    CameraState cam{};

    cam.swapchain  = &swapchain;
    cam.descriptor = &descriptor;

    cam.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    cam.position = {0.0f, 0.0f, -5.0f};

    update(cam, 0, swapchain.get_extent());

    return cam;
}

void camera_system::update(CameraState &cam, uint32_t current_image, VkExtent2D extent)
{
  CAMERA_UBO ubo{};

  glm::mat4 rotation_matrix = glm::toMat4(glm::conjugate(cam.rotation));
  glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), -cam.position);

  ubo.view = rotation_matrix * translation_matrix;

  ubo.proj = glm::perspective(glm::radians(cam.fov),
      extent.width / (float) extent.height, 0.001f, 1000.0f);
  ubo.proj[1][1] *= -1;

  memcpy(cam.descriptor->get_uniform_mapped(current_image), &ubo, sizeof(ubo));
}

void camera_system::move(CameraState& cam, float x, float y, float z, float delta_time)
{
    cam.position += glm::vec3(x, y, z) * cam.speed * delta_time;
}

void camera_system::move(CameraState& cam, const glm::vec3& direction, float delta_time)
{
    cam.position += direction * cam.speed * delta_time;
}

void camera_system::rot(CameraState& cam, float pitch, float yaw, float roll)
{
    cam.yaw   += yaw;
    cam.pitch += pitch;
    cam.pitch = glm::clamp(cam.pitch, -90.0f, 90.0f);

    glm::quat yaw_quat   = glm::angleAxis(glm::radians(cam.yaw),   glm::vec3(0, 1, 0));
    glm::quat pitch_quat = glm::angleAxis(glm::radians(cam.pitch), glm::vec3(1, 0, 0));

    cam.rotation = glm::normalize(yaw_quat * pitch_quat);
}

void camera_system::rotate(CameraState &cam, const glm::quat &quat) 
{
  cam.rotation = quat * cam.rotation;
  cam.rotation = glm::normalize(cam.rotation);
}

glm::vec3 camera_system::get_forward_vector(const CameraState &cam)
{ 
  glm::mat4 rot_mat = glm::toMat4(cam.rotation);
  glm::vec3 forward = glm::vec3(rot_mat[2][0], rot_mat[2][1], rot_mat[2][2]);
  return -glm::normalize(forward);
}

glm::vec3 camera_system::get_right_vector(const CameraState &cam)
{ 
  glm::mat4 rot_mat = glm::toMat4(cam.rotation);
  return glm::normalize(glm::vec3(rot_mat[0][0], rot_mat[0][1], rot_mat[0][2]));
}

glm::vec3 camera_system::get_up_vector(const CameraState &cam)
{ 
  glm::mat4 rot_mat = glm::toMat4(cam.rotation);
  return glm::normalize(glm::vec3(rot_mat[1][0], rot_mat[1][1], rot_mat[1][2]));
}
