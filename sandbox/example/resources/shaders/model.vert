#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragColor;

layout (binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 project;
} camera;

void main() {
    vec4 position = camera.project * camera.view * vec4(inPosition, 1.0);
    gl_Position = vec4(position);
    fragNormal = inNormal;
    fragColor = inColor;
}