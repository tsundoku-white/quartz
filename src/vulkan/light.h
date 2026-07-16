#pragma once

#include "../core/core.h"
#include "src/vulkan/descriptor.h"
#include <cstdint>

struct alignas(16) SUN_UBO {
    glm::vec3 direction;
    float _pad0 = 0.0f;
    glm::vec3 color;
    float intensity;
};

class SunLight 
{
public:
    SunLight(Descriptor &descriptor, glm::vec3 dir = {-0.3f, -1.0f, -0.2f},
              glm::vec3 color = {1.0f, 0.96f, 0.84f},
              float intensity = 1.5f)
        : direction(glm::normalize(dir)), color(color), intensity(intensity),
        m_descriptor(descriptor){}

    void set_direction(glm::vec3 dir) { direction = glm::normalize(dir); }

    SUN_UBO get_ubo_data() const {
        return { direction, 0.0f, color, intensity };
    }

    void update(uint32_t current_frame);

private:
    Descriptor &m_descriptor;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};
