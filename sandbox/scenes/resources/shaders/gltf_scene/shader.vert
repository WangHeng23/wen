#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 project;
} camera;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec4 fragColor;

void main() {
    fragNormal = inPosition;
    fragColor = vec4(1.0);
    gl_Position = camera.project * camera.view * vec4(inPosition / 400, 1.0);
}