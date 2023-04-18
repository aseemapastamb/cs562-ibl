#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
//layout (location = 2) in vec2 tex_coords;
//layout (location = 3) in vec3 tangent;
//layout (location = 4) in vec3 bi_tangent;
layout (location = 2) in ivec4 bone_ids; 
layout (location = 3) in vec4 weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 final_bones_matrices[MAX_BONES];

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
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