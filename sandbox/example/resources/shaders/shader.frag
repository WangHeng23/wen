#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D tex;

void main() {
    // outColor = vec4(fragColor, 1.0);
    outColor = vec4(texture(tex, fragUV * 2.0).rgb, 1.0);
    // outColor = vec4(fragColor * texture(tex, fragUV).rgb, 1.0);
}