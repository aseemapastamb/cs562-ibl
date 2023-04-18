#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec4 

    mat4 S = mat4(0.0f);
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (bone_ids[i] >= 0) {
            S += final_bones_matrices[bone_ids[i]] * weights[i];
        }
    }
    mat3 S_ = transpose(inverse(mat3(S)));
//    TexCoord = tex_coords;
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = normalize(S_ * normal);
    gl_Position = projection * view * model * S * vec4(position, 1.0f);
}