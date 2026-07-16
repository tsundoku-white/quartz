#version 450

layout(binding = 2) uniform sampler2D tex_sampler;

layout(binding = 3) uniform SUN_UBO {
    vec3 direction;   
    vec3 color;
    float intensity;
} sun;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_tex_coord;
layout(location = 2) in vec3 frag_normal;
layout(location = 3) in vec3 frag_world_pos;

layout(location = 0) out vec4 out_color;

void main() {
    vec3 N = normalize(frag_normal);
    vec3 L = normalize(-sun.direction); 

    float diffuse = max(dot(N, L), 0.0);

    vec3 ambient = vec3(0.7); 
    vec3 lighting = ambient + diffuse * sun.color * sun.intensity;

    vec4 tex = texture(tex_sampler, frag_tex_coord);
    out_color = vec4(tex.rgb * lighting, tex.a);
}
