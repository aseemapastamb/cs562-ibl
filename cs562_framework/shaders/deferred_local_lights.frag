#version 330 core

const float PI = 3.1415926538f;

struct LocalLight {
    vec3 position;
    vec3 colour;
    float radius;
};

uniform sampler2D g_world_pos;
uniform sampler2D g_world_norm;
uniform sampler2D g_diffuse_colour;
uniform sampler2D g_specular_colour;

uniform uint buffer_width;
uniform uint buffer_height;

uniform LocalLight local_light;
uniform vec3 view_pos;

uniform bool hard_edges;
uniform bool local_lights_off;

out vec4 FragColor;

// function prototypes
vec3 CalculateLocalLight(LocalLight light, vec3 normal, vec3 view_direction);
float DistributionGGX(vec3 N, vec3 H, float alpha);
float GeometryGGX(vec3 N, vec3 L, vec3 V, vec3 H, float alpha);
float G1GGX(vec3 N, vec3 V, vec3 H, float alpha);
vec3 ReflectionFresnelSchlick(float LH, vec3 Ks);

void main() {
    vec3 colour = vec3(0);

    if (local_lights_off == false) {
        // read in values from g buffer
        vec2 uv = gl_FragCoord.xy / vec2(buffer_width, buffer_height);
        vec4 world_pos = texture(g_world_pos, uv);
        vec3 normal_vec = texture(g_world_norm, uv).xyz;
        vec4 Kd_and_w = texture(g_diffuse_colour, uv);
        vec3 Kd = Kd_and_w.xyz;

        if (Kd_and_w.w == -100.0f) {   // indicates skydome so don't use regular lighting
            // tone mapping
            Kd = pow(Kd, vec3(1.0f / 2.2f));
            FragColor = vec4(Kd, 1.0f);
            return;
        }

        vec4 Ks_and_rough = texture(g_specular_colour, uv);
        vec3 Ks = Ks_and_rough.xyz;
        float alpha = Ks_and_rough.w; // roughness

        // add a little light to see range of light influence
        if (hard_edges) {
            colour += 0.1f * local_light.colour;
        }

        // lighting calculation only if pixel is in radius of light
        vec3 L = local_light.position - world_pos.xyz;
        float dist_sq = dot(L, L);
        float radius_sq = local_light.radius * local_light.radius;
        if (dist_sq < radius_sq) {
            // values needed for light calculation
            vec3 N = normalize(normal_vec);
                 L = normalize(L);
            vec3 V = normalize(view_pos - world_pos.xyz);
            vec3 H = normalize(L + V);

            float LH = dot(L, H);

//            // Phong (TODO: BRDF)
//            vec3 ambient = 0.1f * Kd;
//            vec3 diffuse = (LN * Kd) * local_light.colour;
//            float spec = pow(max(dot(V, R), 0.0f), alpha);
//            vec3 specular = (spec * Ks) * local_light.colour;

            // BRDF
            vec3 ambient = 0.1f * Kd;
            vec3 diffuse = local_light.colour * Kd / PI;
            vec3 specular = local_light.colour * ReflectionFresnelSchlick(LH, Ks) * DistributionGGX(N, H, alpha) * GeometryGGX(N, L, V, H, alpha)
                            / (4.0f * abs(dot(L, N)) * abs(dot(V, N)));

            // light falls off as distance increases
            float attenuation = (1.0f / dist_sq) - (1.0f / radius_sq);
            ambient *= attenuation;
            diffuse *= attenuation;
            specular *= attenuation;

            // combine
            colour += ambient + diffuse + specular;
        }
    }
    
    // tone mapping
    colour = pow(colour, vec3(1.0f / 2.2f));
    FragColor = vec4(colour, 1.0f);
}

float DistributionGGX(vec3 N, vec3 H, float alpha) {
    float HN = dot(H, N);
    float NH = dot(N, H);

    float char_factor = 1.0f;
    if (HN <= 0.0f) {
        char_factor = 0.0f;
    }

    float tan_theta_m = sqrt(1.0f - (HN * HN)) / HN;
    float tan_sq = tan_theta_m * tan_theta_m;

    float alpha_sq = alpha * alpha;

    float D_GGX = char_factor * alpha_sq / (PI * pow(NH, 4.0f) * pow(alpha_sq + tan_sq, 2.0f));

    return D_GGX;
}

float GeometryGGX(vec3 N, vec3 L, vec3 V, vec3 H, float alpha) {
    float G_Smith_GGX = G1GGX(N, L, H, alpha) * G1GGX(N, V, H, alpha);

    return G_Smith_GGX;
}

float G1GGX(vec3 N, vec3 V, vec3 H, float alpha) {
    float VH = dot(V, H);
    float VN = dot(V, N);

    if (VN > 1.0f) { return 1.0f; }

    float VH_by_VN = VH / VN;
    float char_factor = 1.0f;
    if (VH_by_VN <= 0.0f) {
        char_factor = 0.0f;
    }

    float tan_theta_v = sqrt(1.0f - (VN * VN)) / VN;
    float tan_sq = tan_theta_v * tan_theta_v;
    if (tan_sq <= 0.0001f) { return 1.0f; }

    float alpha_sq = alpha * alpha;

    float G1 = char_factor * 2.0f / (1.0f + (sqrt(1.0f + (alpha_sq * tan_sq))));

    return G1;
}

vec3 ReflectionFresnelSchlick(float LH, vec3 Ks) {
    vec3 F = Ks + ((1.0f - Ks) * pow(1.0f - abs(LH), 5.0f));

    if (F.x < 0.0f) { F.x = 0.0f; }
    if (F.y < 0.0f) { F.y = 0.0f; }
    if (F.z < 0.0f) { F.z = 0.0f; }

    return F;
}