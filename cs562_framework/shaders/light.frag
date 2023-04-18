#version 330 core

struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec4 world_pos;
in vec3 world_norm;

uniform vec3 view_pos;
uniform DirectionalLight global_light;
uniform Material material;

out vec4 FragColor;

// function prototypes
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_direction);

void main() {
    // properties
    vec3 norm = normalize(world_norm);
    vec3 view_dir = normalize(view_pos - world_pos.xyz);

    vec3 result = CalculateDirectionalLight(global_light, norm, view_dir);

    FragColor = vec4(result, 1.0);
}

// Phong
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_direction) {
    // ambient
    vec3 ambient = light.ambient * material.diffuse;
    
    // diffuse shading
    vec3 light_dir = normalize(light.position - world_pos.xyz);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * light.diffuse * material.diffuse;

    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_direction, reflect_dir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * material.specular;

    // combine
    return (ambient + diffuse + specular);
}