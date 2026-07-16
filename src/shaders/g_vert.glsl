#version 450

layout(binding = 0) uniform CAMERA_UBO {
    mat4 view;
    mat4 proj;
} camera_ubo;

layout(binding = 1) uniform MESH_UBO {
    mat4 model;
} mesh_ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_tex_coord;
layout(location = 3) in vec3 in_normal;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_tex_coord;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec3 frag_world_pos;

void main() {
    vec4 world_pos = mesh_ubo.model * vec4(in_position, 1.0);
    gl_Position = camera_ubo.proj * camera_ubo.view * world_pos;

    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
    frag_world_pos = world_pos.xyz;

    frag_normal = mat3(mesh_ubo.model) * in_normal;
}
