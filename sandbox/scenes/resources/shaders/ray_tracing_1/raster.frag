#version 450

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec3 fragColor;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PointLight {
    vec3 position;
    vec3 color;
    float intensity; 
} light;

void main() {
    vec3 l = normalize(light.position - fragPosition);
    float len = length(light.position - fragPosition);
    vec3 normal = normalize(fragNormal);

    outColor = vec4(vec3(0.05) + max(dot(l, normal), 0) * light.color * light.intensity * 0.8 / len / len, 1);
}
