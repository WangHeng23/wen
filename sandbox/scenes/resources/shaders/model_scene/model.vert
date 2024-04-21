#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 offset;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragColor;

layout (binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 project;
} camera;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};
layout(binding = 1) uniform LightUniform {
    PointLight pointLight[8];
    uint lightCount;
} light;

void main() {
    vec3 pos = offset + inPosition;
    gl_Position = camera.project * camera.view * vec4(pos, 1);

    vec3 cool = vec3(0, 0, 0.55) + inColor / 4;
    vec3 warm = vec3(0.3, 0.3, 0) + inColor / 4;
    vec3 highLight = vec3(2, 2, 2);
    vec3 color = cool / 2;
    for (uint i = 0; i < light.lightCount; i ++) {
        vec3 N = normalize(inNormal);
        vec3 V = normalize(camera.position - pos);
        vec3 L = normalize(light.pointLight[i].position - pos);
        vec3 R = reflect(-L, N);
        float s = clamp(100 * dot(R, V) - 97, 0, 1);
        color += clamp(dot(L, N), 0, 1) * light.pointLight[i].color * mix(warm, highLight, s) * light.pointLight[i].intensity;
    }
    fragColor = color;

    fragNormal = inNormal;
}