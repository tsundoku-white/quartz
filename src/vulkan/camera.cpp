#include "camera.h"
#include "swapchain.h"
#include "descriptor.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <cstring>

Camera::Camera(VulkanSwapchain &swapchain, Descriptor &descriptor) 
  : m_swapchain(swapchain), m_descriptor(descriptor)
{
  m_quart_rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  update(0, m_swapchain.get_extent());
}

Camera::~Camera()
{
  return;
}

void Camera::update(uint32_t current_image, VkExtent2D extent)
{
  CAMERA_UBO ubo{};

  glm::mat4 rotation_matrix = glm::toMat4(glm::conjugate(m_quart_rot));
  glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), -m_loc);

  ubo.view = rotation_matrix * translation_matrix;

  ubo.proj = glm::perspective(glm::radians(m_fov),
      extent.width / (float) extent.height, 0.001f, 1000.0f);
  ubo.proj[1][1] *= -1;

  memcpy(m_descriptor.get_uniform_mapped(current_image), &ubo, sizeof(ubo));
}

void Camera::move(const glm::vec3& direction, float delta_time)
{
  m_loc += direction * m_speed * delta_time;
}

void Camera::rot(float pitch, float yaw, float roll)
{
  m_yaw   += yaw;
  m_pitch += pitch;
  m_pitch = glm::clamp(m_pitch, -90.0f, 90.0f);

  glm::quat yaw_quat   = glm::angleAxis(glm::radians(m_yaw),   glm::vec3(0, 1, 0));
  glm::quat pitch_quat = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1, 0, 0));

  m_quart_rot = glm::normalize(yaw_quat * pitch_quat);
}

void Camera::rotate(const glm::quat& quat)
{
  m_quart_rot = quat * m_quart_rot;
  m_quart_rot = glm::normalize(m_quart_rot);
}

glm::vec3 Camera::get_forward_vector() const
{ 
  // Forward is the -Z axis of the rotation matrix
  glm::mat4 rot_mat = glm::toMat4(m_quart_rot);
  glm::vec3 forward = glm::vec3(rot_mat[2][0], rot_mat[2][1], rot_mat[2][2]);
  return -glm::normalize(forward);
}

glm::vec3 Camera::get_right_vector() const
{ 
  // Right is the +X axis of the rotation matrix
  glm::mat4 rot_mat = glm::toMat4(m_quart_rot);
  return glm::normalize(glm::vec3(rot_mat[0][0], rot_mat[0][1], rot_mat[0][2]));
}

glm::vec3 Camera::get_up_vector() const
{ 
  // Up is the +Y axis of the rotation matrix
  glm::mat4 rot_mat = glm::toMat4(m_quart_rot);
  return glm::normalize(glm::vec3(rot_mat[1][0], rot_mat[1][1], rot_mat[1][2]));
}
