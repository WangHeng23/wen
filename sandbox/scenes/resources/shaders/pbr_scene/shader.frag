#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CameraUniform {
    vec3 position;
    mat4 view;
    mat4 project;
} camera;

layout(binding = 1) uniform MaterialUniform {
    vec3 color;
    float roughness;
    float metallic;
    float reflectance;
} material;

struct PointLight {
    vec3 position;
    vec3 color;
};

layout(binding = 2) uniform LightsUniform {
    vec3 direction;            // 平行光
    vec3 color;                // 平行光颜色
    PointLight pointLights[3]; // 点光源
} lights;


const float PI = 3.1415926535;
const float SPEC_ADD = 1.1;

float D_GGX_TR(float NoH, float roughness2) {
    float NoH2 = NoH * NoH;

    float nom = roughness2;
    float denom = NoH2 * (roughness2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySmith(float NoL, float NoV, float roughness2) {
    float roughness = sqrt(roughness2);
    float k1 = pow(roughness + 1.0, 2.0) / 8.0;
    float k2 = roughness2 / 2.0;
    float ggx1 = NoV / (NoV * (1.0 - k1) + k1);
    float ggx2 = NoL / (NoL * (1.0 - k1) + k1);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(vec3 F0, float cosTheta) {
    return F0 + (1 - F0) * pow(clamp(1 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 computeDirectionLight(vec3 direction, vec3 color, vec3 N, vec3 V, vec3 R0, float roughness2) {
    vec3 L = normalize(direction);
    vec3 H = normalize(L + V);     // 半程向量
    float NoL = max(dot(N, L), 0); // 入射角
    float NoV = max(dot(N, V), 0); // 视角
    float NoH = max(dot(N, H), 0); // 法线和半程向量夹角
    float LoH = max(dot(L, H), 0); // 入射光线和半程向量夹角

    // 漫反射
    vec3 diffuseColor = material.color * (1 - material.metallic) / PI;
    // 镜面反射
    vec3 specColor = fresnelSchlick(R0, LoH) * GeometrySmith(NoL, NoV, roughness2) * D_GGX_TR(NoH, roughness2);

    return (diffuseColor + specColor * SPEC_ADD) * NoL * color;
}

vec3 computePointLight(vec3 position, vec3 color, vec3 pos, vec3 N, vec3 V, vec3 R0, float roughness2) {
    vec3 L = normalize(position - pos);
    float distance = length(position - pos);
    return computeDirectionLight(L, color, N, V, R0, roughness2) / (distance * distance);
}

void main() {
    // 转换PBR参数
    float roughness = material.roughness * material.roughness;
    float reflectance = 0.16 * material.reflectance * material.reflectance;
    // 基础反射率
    vec3 R0 = material.color * material.metallic + (reflectance * (1 - material.metallic));

    // 法线(片段法线方向)
    vec3 N = normalize(fragNormal);
    // 视线(片段位置到相机位置的方向)
    vec3 V = normalize(camera.position - fragPosition);

    float roughness2 = clamp(roughness * roughness, 0.001, 0.999);

    vec3 color = computeDirectionLight(-lights.direction, lights.color, N, V, R0, roughness2);
    for (int i = 0; i < 3; i ++) {
        color += computePointLight(lights.pointLights[i].position, lights.pointLights[i].color, fragPosition, N, V, R0, roughness2);
    }

    color = color / (color + 1);
    color = pow(color, vec3(1 / 2.2));

    outColor = vec4(color, 1);
}
