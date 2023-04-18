#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float depth;

void main() {
    vec4 shadow_pos = projection * view * model * vec4(aPos, 1.0f);
    depth = shadow_pos.w;
    gl_Position = shadow_pos;
}