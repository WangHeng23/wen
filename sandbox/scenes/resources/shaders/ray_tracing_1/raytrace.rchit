#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : enable

#include "ray.glsl"
#include <RayTracing>

layout (binding = 0, set = 0) uniform accelerationStructureEXT tlas;

layout (location = 0) rayPayloadInEXT HitPayload prd;
layout (location = 1) rayPayloadEXT bool shadow;

layout (binding = 2, set = 0, scalar) buffer InstanceInfo_ {
    InstanceAddressInfo infos[];
} info_;

layout(push_constant) uniform PointLight {
    vec3 position;
    vec3 color;
    float intensity; 
} light;

layout(buffer_reference, scalar) buffer Vertices {
    Vertex vertices[];
};
layout(buffer_reference, scalar) buffer Indices {
    Index indices[];
};

// 质心坐标
hitAttributeEXT vec3 attribs;

void main() {
    InstanceAddressInfo info = info_.infos[gl_InstanceCustomIndexEXT];

    Vertices vertices = Vertices(info.vertexBufferAddress);
    Indices indices = Indices(info.indexBufferAddress);
    Index idxs = indices.indices[gl_PrimitiveID];

    // 质心权重
    vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    Vertex v0 = vertices.vertices[idxs.i0];
    Vertex v1 = vertices.vertices[idxs.i1];
    Vertex v2 = vertices.vertices[idxs.i2];
    // 交点坐标
    vec3 position = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
    vec3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);

    vec3 l = normalize(light.position - position);
    float len = length(light.position - position);

    shadow = true;
    uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    traceRayEXT(
        tlas,
        rayFlags,
        0xff,
        0, 0,
        1,
        position,
        0.0001,
        l,
        len,
        1
    );

    prd.value = max(dot(l, normal), 0) * light.color * light.intensity / len / len;

    if (shadow) {
        prd.value *= 0.3;
    }    
}
