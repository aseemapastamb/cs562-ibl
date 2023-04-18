#version 330 core

const float PI = 3.1415926538f;

struct Material {
    vec3 diffuse;
    vec3 specular;
    float roughness;
};

in vec4 world_pos;
in vec3 world_norm;
in vec3 a_pos;

uniform Material material;

uniform bool is_skydome;
uniform sampler2D skydome;

layout (location = 0) out vec4 g_world_pos;
layout (location = 1) out vec4 g_world_norm;
layout (location = 2) out vec4 g_diffuse_colour;
layout (location = 3) out vec4 g_specular_colour;

void main() {
    if (is_skydome) {
        // get uv from direction vector N
        vec2 uv = vec2(0.5f - (atan(a_pos.x, a_pos.z) / (2.0f * PI)), acos(a_pos.y) / PI);

        // g-buffer 3: per fragment diffuse colour      | setting w to -100.0f to indicate skydome
        vec3 diffuse = texture(skydome, uv).xyz;
        diffuse = pow(diffuse, vec3(2.2f));
        g_diffuse_colour = vec4(diffuse, -100.0f);
        return;
    }
    else {
        // g-buffer 1: fragment position
        g_world_pos = world_pos;
        // g-buffer 2: per fragment normals
        g_world_norm = vec4(normalize(world_norm), 1.0f);
        // g-buffer 3: per fragment diffuse colour
        vec3 diffuse = pow(material.diffuse, vec3(2.2f));
        g_diffuse_colour = vec4(diffuse, 1.0f);
        // g-buffer 4: per fragment specular colour
        vec3 specular = pow(material.specular, vec3(2.2f));
        g_specular_colour = vec4(specular, material.roughness);
    }
}