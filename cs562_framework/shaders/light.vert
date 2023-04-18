#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 world_pos;
out vec3 world_norm;

void main() {
    world_pos = model * vec4(aPos, 1.0);
    mat3 normal_matrix = transpose(inverse(mat3(model)));
    world_norm = normal_matrix * aNorm;

    gl_Position = projection * view * world_pos;

    world_pos.w = gl_Position.w;
}