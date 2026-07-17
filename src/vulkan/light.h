#pragma once
#include "../core/core.h"
#include "descriptor.h"

class SunLight {
public:
    SunLight(Descriptor& descriptor) : m_descriptor(descriptor) {
        direction = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
        color = glm::vec4(1.0f, 0.9f, 0.8f, 0.0f);
        intensity = 0.5f;
    }

    void update(uint32_t current_frame); 

    SUN_UBO get_ubo_data() const {
        SUN_UBO ubo{};
        ubo.direction = direction;
        ubo.color = color;
        ubo.intensity = intensity;
        return ubo;
    }

private:
    Descriptor& m_descriptor;
    glm::vec4 direction;
    glm::vec4 color;
    float intensity;
};
