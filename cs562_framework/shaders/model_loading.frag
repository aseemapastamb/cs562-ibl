#version 330 core

out vec4 FragColor;

struct Material {
//    sampler2D diffuse;
//    sampler2D specular;
//    sampler2D normal;
//    sampler2D height;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//struct PointLight {
//    vec3 position;
//
//    float constant;
//    float linear;
//    float quadratic;
//
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;
//};

uniform vec3 viewPos;
uniform DirectionalLight directionalLight;
//uniform PointLight pointLight;
uniform Material material;

// texture samplers
//uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;

// function prototypes
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_direction);
//vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 view_direction);

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

void main() {
    // properties
    vec3 norm = normalize(Normal);
//    vec3 norm = normalize(texture(material.normal, TexCoord).rgb);
    vec3 view_dir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0f, 0.0f, 0.0f);

    // directional lighting
    result = CalculateDirectionalLight(directionalLight, norm, view_dir);
    // point lights
//    result += CalculatePointLight(pointLight, norm, FragPos, view_dir);
    
    FragColor = vec4(result, 1.0);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_direction) {
    vec3 light_dir = normalize(-light.direction);
//    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;
    vec3 ambient = light.ambient;
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0f);
//    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_direction, reflect_dir), 0.0f), material.shininess);
//    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;
    vec3 specular = light.specular * spec * material.specular;
    // combine
    return (ambient + diffuse + specular);
}

//vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 view_direction) {
//    vec3 light_dir = normalize(light.position - fragPos);
//    // ambient
////    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;
//    vec3 ambient = light.ambient;
//    // diffuse shading
//    float diff = max(dot(normal, light_dir), 0.0f);
////    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
//    vec3 diffuse = light.diffuse * diff * material.diffuse;
//    // specular shading
//    vec3 reflect_dir = reflect(-light_dir, normal);
//    float spec = pow(max(dot(view_direction, reflect_dir), 0.0f), material.shininess);
////    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;
//    vec3 specular = light.specular * spec * material.specular;
//    // attenuation
//    float distance = length(light.position - fragPos);
//    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
//    ambient *= attenuation;
//    diffuse *= attenuation;
//    specular *= attenuation;
//    // combine
//    return (ambient + diffuse + specular);
//}