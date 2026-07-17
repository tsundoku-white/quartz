#include "light.h"
#include "descriptor.h"
#include "camera.h"
#include "mesh.h"

void SunLight::update(uint32_t current_frame)
{
  SUN_UBO sun_ubo{};
  sun_ubo.direction = direction;
  sun_ubo.color = color;
  sun_ubo.intensity = intensity;

  void* data = m_descriptor.get_uniform_mapped(current_frame);
  if (data) {
    memcpy(static_cast<char*>(data) + sizeof(CAMERA_UBO) + sizeof(MESH_UBO), 
        &sun_ubo, sizeof(SUN_UBO));
  }
}
