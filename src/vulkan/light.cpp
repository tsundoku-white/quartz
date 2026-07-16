#include "light.h"
#include <cstdint>
#include "descriptor.h"
#include "camera.h"
#include "mesh.h"

void SunLight::update(uint32_t current_frame)
{
    void* data = m_descriptor.get_uniform_mapped(current_frame);
    if (!data) {
        throw std::runtime_error("Failed to get mapped uniform buffer memory");
    }

    SUN_UBO ubo = get_ubo_data();
    memcpy(static_cast<char*>(data) + sizeof(CAMERA_UBO) + sizeof(MESH_UBO),
           &ubo, sizeof(SUN_UBO));
}
