#version 330 core

out vec4 FragColor;

in vec2 texCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    // lerp between both textures (80/20)
    FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2);
}