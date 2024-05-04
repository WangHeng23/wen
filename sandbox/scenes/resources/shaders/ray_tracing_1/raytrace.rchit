#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : enable

#include "ray.glsl"
#include <RayTracing>

layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

// 在这个最近命中着色器中，要计算返回rgen着色器的prd.value值
layout(location = 0) rayPayloadInEXT HitPayload prd;
// 另外一个阴影负载
layout(location = 1) rayPayloadEXT bool shadow;

// storage buffer
layout(binding = 2, set = 0, scalar) buffer InstanceInfo_ {
    InstanceAddressInfo infos[];
} instanceInfo;

layout(push_constant) uniform PushConstant {
    vec3 position;
    vec3 color;
    float intensity; 
} light;

// 物体的位置
layout(buffer_reference, scalar) buffer Vertices {
    Vertex vertices[];
};
// 三角形的索引
layout(buffer_reference, scalar) buffer Indices {
    Index indices[];
};

// 质心坐标
hitAttributeEXT vec3 attribs;

// gl_InstanceCustomIndexEXT 用于告诉我们光线和哪一个物体相交
// gl_PrimitiveID 用于找到被击中三角形的顶点信息
void main() {
    InstanceAddressInfo info = instanceInfo.infos[gl_InstanceCustomIndexEXT];
    Vertices vertices = Vertices(info.vertexBufferAddress);
    Indices indices = Indices(info.indexBufferAddress);

    // 三角形索引
    Index idxs = indices.indices[gl_PrimitiveID];
    // 三角形顶点
    Vertex v0 = vertices.vertices[idxs.i0];
    Vertex v1 = vertices.vertices[idxs.i1];
    Vertex v2 = vertices.vertices[idxs.i2];

    // 质心权重
    vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    // 命中点坐标(模型空间 -> 世界空间)
    vec3 position = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
    position = (gl_ObjectToWorldEXT * vec4(position, 1.0)).xyz;
    // 命中点法线(模型空间 -> 世界空间)
    vec3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
    normal = normalize((gl_ObjectToWorldEXT * vec4(normal, 0.0)).xyz);

    // gl_RayFlagsSkipClosestHitShaderKHR ：将不会调用最近命中着色器，只会调用未命中着色器
    // gl_RayFlagsOpaqueKHR ：将不会调用任意命中着色器，所以所有的对象都是不透明的
    // gl_RayFlagsTerminateOnFirstHitKHR ：使用第一个交点就好
    // 不调用命中着色器，因为没有必要
    // 打到第一个点，就确定是阴影了
    uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

    shadow = true;

    vec3 l = normalize(light.position - position);
    float len = length(light.position - position);

    /*
        !!! 命中点是相机发射光线的击打点
        1. 从最近命中着色器中发射光线，新增加的光线类型需要增加一个新的未命中着色器(shadow.miss)
        2. 当阴影光线(从击中点到光源)没有和任何几何体相交的话(就是该点可以被光源发出的光打到)
           将会调用第二个未命中着色器，其只是表示未发生遮挡(将 shadow 赋值为 false)
        3. 阴影光线击中物体，表明该区域为阴影，不改变 shadow(true)
    */
    traceRayEXT(
        tlas,
        rayFlags,
        0xff,
        0,
        0,
        1,         // missIndex = 1 -> shadow.miss
        position,  // 阴影光线从命中点发射光线
        0.0001,
        l,         // 方向从命中点指向光源
        len,       // 命中点到光源的长度
        1          // payload (location = 1) -> shadow
    );

    // 计算返回rgen着色器的prd.value值
    prd.value = max(dot(l, normal), 0) * light.color * light.intensity / len / len;

    if (shadow) {
        prd.value *= 0.3;
    }    
}
