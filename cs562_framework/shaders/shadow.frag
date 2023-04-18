#version 330 core

in float depth;

uniform float min_depth;
uniform float max_depth;

layout (location = 0) out vec4 FragColor;

void main() {
    // convert depth into relative depth
    float relative_depth = (depth - min_depth) / (max_depth - min_depth);
//    FragColor.xyz = vec3(relative_depth);
//    FragColor.xyz = vec3(shadow_pos.w / 100.0f);
//    FragColor.w = shadow_pos.w;
    FragColor = vec4(relative_depth,
                     pow(relative_depth, 2),
                     pow(relative_depth, 3),
                     pow(relative_depth, 4));
//    FragColor = vec4(depth,
//                     pow(depth, 2),
//                     pow(depth, 3),
//                     pow(depth, 4));
}