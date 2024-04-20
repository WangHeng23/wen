#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 offset;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

layout (binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 project;
} camera;

void main() {
    vec4 pos = camera.project * camera.view * vec4(position, 0.0, 1.0);
    gl_Position = vec4(pos + vec4(offset * 1.5, 1.0));
    fragColor = color;
    fragUV = uv;
}