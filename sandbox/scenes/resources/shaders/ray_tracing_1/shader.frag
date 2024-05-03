#version 450

layout(binding = 0) uniform sampler2D image;

layout(binding = 0, set = 1) uniform Info {
    vec2 windowSize;
    vec3 clearColor;
} info;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / info.windowSize;
    uv.y = 1 - uv.y;
    outColor = texture(image, uv);
}
