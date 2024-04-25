#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 3) in vec3 offset;
layout(location = 4) in float scale;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;

layout(binding = 0) uniform CameraUniform {
    vec3 position;
    mat4 view;
    mat4 project;
} camera;

void main() {
    fragPosition = inPosition * scale + offset;
    fragNormal = inNormal;
    fragColor = inColor;
    gl_Position = camera.project * camera.view * vec4(fragPosition, 1);
}