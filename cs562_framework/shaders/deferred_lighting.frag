#version 330 core

const float PI = 3.1415926538f;

struct DirectionalLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// G-Buffer
uniform sampler2D g_world_pos;
uniform sampler2D g_world_norm;
uniform sampler2D g_diffuse_colour;
uniform sampler2D g_specular_colour;

// Shadow Map
uniform sampler2D shadow_map;

// IBL
uniform bool is_ibl;
uniform sampler2D skydome_map;
uniform sampler2D irradiance_map;
uniform HammersleyBlock {
    uint n;
    float hammersley[2 * 50];
} hammersley_block;
uniform float exposure;

uniform uint buffer_width;
uniform uint buffer_height;

uniform DirectionalLight global_light;
uniform vec3 view_pos;
uniform mat4 shadow_mat;

uniform float min_depth;
uniform float max_depth;

uniform bool global_light_off;
uniform bool var_or_msm;

out vec4 FragColor;

// function prototypes
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_direction);
bool InNormalRange(float val);
float VarianceMethod(vec4 moments, float rel_pixel_depth);
float Hamburger4MSMMethod(vec4 moments, float rel_pixel_depth);
vec3 CholeskyDecomposition(vec4 b_dash, float rel_pixel_depth);
float DistributionGGX(vec3 N, vec3 H, float alpha);
float GeometryGGX(vec3 N, vec3 L, vec3 V, vec3 H, float alpha);
float G1GGX(vec3 N, vec3 V, vec3 H, float alpha);
vec3 ReflectionFresnelSchlick(float LH, vec3 Ks);

void main() {
    vec3 colour = vec3(0);

    if (global_light_off == false) {
        // read in values from g buffer
        vec2 uv = gl_FragCoord.xy / vec2(buffer_width, buffer_height);
        vec3 world_pos = texture(g_world_pos, uv).xyz;
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
        float roughness = Ks_and_rough.w; // roughness
        float alpha = roughness;

        // values needed for light calculation
        vec3 N = normalize(normal_vec);
        vec3 L = normalize(global_light.position - world_pos);
        vec3 V = normalize(view_pos - world_pos);
//        vec3 R = normalize(reflect(-L, N));
        
//        // Phong (TODO: BRDF)
//        vec3 ambient = global_light.ambient * Kd;
//        vec3 diffuse = (LN * Kd) * Ia;
//        float spec = pow(max(dot(V, R), 0.0f), alpha);
//        vec3 specular = (spec * Ks) * Ii;
        
        // BRDF
        if (is_ibl) {
            // get uv from direction vector N
            uv = vec2(0.5f - (atan(N.x, N.z) / (2 * PI)), acos(N.y) / PI);
            vec3 irradiance = texture(irradiance_map, uv).rgb;
            vec3 diffuse = irradiance * Kd / PI;
            vec3 specular = vec3(0);

            // rotation frame is reflection vector R
            vec3 R = (2.0f * dot(N, V) * N) - V;
            // A = Yaxis X R
            vec3 A = normalize(cross(vec3(0.0f, 1.0f, 0.0f), R));
            // B = R X A
            vec3 B = normalize(cross(R, A));

            for (int i = 0; i < int(hammersley_block.n) * 2; i += 2) {
                // random numbers from hammersley block
                float rand1 = hammersley_block.hammersley[i];
                float rand2 = hammersley_block.hammersley[i + 1];

                // theta from GGX BRDF
                float theta = atan(alpha * sqrt(rand2) / sqrt(1.0f - rand2));
                // skew the random numbers to match DistributionGGX
                uv = vec2(rand1, theta / PI);

                // get direction vector from texture coordinates
                L = vec3(cos(2.0f * PI * (0.5f - uv.x)) * sin(PI * uv.y),
                         sin(2.0f * PI * (0.5f - uv.x)) * sin(PI * uv.y),
                         cos(PI * uv.y));

                // omega_k = rotation applied to L
                vec3 omega_k = normalize((L.x * A) + (L.y * B) + (L.z * R));

                vec3 H = normalize(omega_k + V);
                float omega_k_dot_H = dot(omega_k, H);

                // For each wk , evaluate the incoming light Li (wk) by accessing the HDR image for each direction
                // vector wk . For the “filter” step of the paper, read this texel from the MIPMAP at a carefully
                // calculated level
                float level = (0.5f * log2(float(buffer_width * buffer_height) / float(hammersley_block.n))) -
                              (0.5f * log2(DistributionGGX(N, H, alpha) / 4.0f));

                uv = vec2(0.5f - (atan(omega_k.x, omega_k.z) / (2 * PI)), acos(omega_k.y) / PI);
                vec3 Li = textureLod(skydome_map, uv, level).rgb;

                specular += ReflectionFresnelSchlick(omega_k_dot_H, Ks) * GeometryGGX(N, omega_k, V, H, alpha) * Li * cos(theta)
                            / (4.0f * abs(dot(omega_k, N)) * abs(dot(V, N)));
            }
            // average
            specular /= hammersley_block.n;

            colour += (diffuse + specular);
        }
        else {
            vec3 Ii = global_light.specular;
            vec3 Ia = global_light.diffuse;

            // shadow calculation (0 = shadow, 1 = not in shadow)
            float in_shadow = 0.0f;

            vec4 shadow_pos = shadow_mat * vec4(world_pos, 1.0f);
            vec2 shadow_index = shadow_pos.xy / shadow_pos.w;

            if (shadow_pos.w > 0 &&
                InNormalRange(shadow_index.x) &&
                InNormalRange(shadow_index.y)) {

                float rel_pixel_depth = (shadow_pos.w - min_depth) / (max_depth - min_depth);
                vec4 moments = texture(shadow_map, shadow_index);

                if (var_or_msm) {
                    // Variance method
                    in_shadow = VarianceMethod(moments, rel_pixel_depth);
                }
                else {
                    // Hamburger 4-MSM method
                    in_shadow = 1.0f - Hamburger4MSMMethod(moments, rel_pixel_depth);
                }
            }
            
            vec3 H = normalize(L + V);
            float LH = dot(L, H);

            vec3 ambient = global_light.ambient * Kd;
            vec3 diffuse = Ia * Kd / PI;
            vec3 specular = Ii * ReflectionFresnelSchlick(LH, Ks) * DistributionGGX(N, H, alpha) * GeometryGGX(N, L, V, H, alpha)
                            / (4.0f * abs(dot(L, N)) * abs(dot(V, N)));
        
            colour += ambient + in_shadow * (diffuse + specular);
        }
    }

    // tone mapping
    colour = exposure * colour / ((exposure * colour) + vec3(1.0f));
    colour = pow(colour, vec3(1.0f / 2.2f));
    FragColor = vec4(colour, 1.0f);
}

bool InNormalRange(float val) {
    return (val >= 0.0f && val <= 1.0f);
}

float VarianceMethod(vec4 moments, float rel_pixel_depth) {
    float rel_light_depth = moments.x;

    float res = 1.0f;
    if (rel_pixel_depth > rel_light_depth) {
        float mean = moments.x;
        float variance = moments.y - pow(mean, 2);

        res = variance / (variance + pow(rel_pixel_depth - mean, 2));
    }

    return res;
}

float Hamburger4MSMMethod(vec4 moments, float rel_pixel_depth) {
    float bias = 0.0001f;
    vec4 b_dash = ((1.0f - bias) * moments) + (bias * vec4(0.5f));

    // c = (c1, c2, c3)
    vec3 c = CholeskyDecomposition(b_dash, rel_pixel_depth);

    // in quadratic equation, a = c3, b = c2, c = c1
    float sqrt_det = sqrt((c.y * c.y) - (4.0f * c.z * c.x));

    // roots = z2, z3
    float z2 = 0.0f;
    float z3 = 0.0f;

    z2 = (-c.y - sqrt_det) / (2.0f * c.z);
    // z2 should always be the smaller root
    if (c.z < 0.0f) {
        z3 = z2;
        z2 = (-c.y + sqrt_det) / (2.0f * c.z);
    }
    else {
        z3 = (-c.y + sqrt_det) / (2.0f * c.z);
    }

    float G = 0.0f;
    if (rel_pixel_depth <= z2) {
        G = 0.0f;
    }
    else if (rel_pixel_depth <= z3) {
        G = ((rel_pixel_depth * z3) - (b_dash.x * (rel_pixel_depth + z3)) + (b_dash.y))
            / ((z3 - z2) * (rel_pixel_depth - z2));
    }
    else {
        G = 1 -
            (((z2 * z3) - (b_dash.x * (z2 + z3)) + (b_dash.y))
             / ((rel_pixel_depth - z2) * (rel_pixel_depth - z3)));
    }

    return G;
}

vec3 CholeskyDecomposition(vec4 b_dash, float rel_pixel_depth) {
    float m11 = 1.0f;
    float m12 = b_dash.x;
    float m13 = b_dash.y;
    float m22 = b_dash.y;
    float m23 = b_dash.z;
    float m33 = b_dash.w;

    float z1 = 1.0f;
    float z2 = rel_pixel_depth;
    float z3 = rel_pixel_depth * rel_pixel_depth;

    float a = 1.0f; //sqrt(m11);
//    if (a <= 0.0f) {
//        a = 0.0001f;
//    }

    float b = m12 / a;
    float c = m13 / a;

    float d = sqrt(m22 - (b * b));
    if (d <= 0.0f) {
        d = 0.0001f;
    }

    float e = (m23 - (b * c)) / d;
    float f = sqrt(m33 - (c * c) - (e * e));
    if (f <= 0.0f) {
        f = 0.0001f;
    }

    float c1_hat = z1 / a;
    float c2_hat = (z2 - (b * c1_hat)) / d;
    float c3_hat = (z3 - (c * c1_hat) - (e * c2_hat)) / f;

    float c3 = c3_hat / f;
    float c2 = (c2_hat - (e * c3)) / d;
    float c1 = (c1_hat - (b * c2) - (c * c3)) / a;

    return vec3(c1, c2, c3);
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