#include "ray_tracing/gltf/gltf_primitive.hpp"
#include "ray_tracing/gltf/gltf_scene.hpp"
#include "core/logger.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace wen {

GLTFPrimitive::GLTFPrimitive(GLTFScene& scene, const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::vector<std::string>& attrs) : scene_(scene) {
    data_.firstVertex = scene.vertices.size();
    data_.firstIndex = scene.indices.size();
    data_.materialIndex = primitive.material;

    auto getAccessorData = [&](const std::string& name, auto fun) {
        if (primitive.attributes.find(name) == primitive.attributes.end()) {
            WEN_WARN("primitive has no attribute {}", name)
            return static_cast<const void*>(nullptr);
        }
        const auto& accessor = model.accessors[primitive.attributes.find(name)->second];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        fun(accessor);
        return reinterpret_cast<const void*>(model.buffers[bufferView.buffer].data.data() + bufferView.byteOffset + accessor.byteOffset);
    };

    auto* positionPtr = static_cast<const float*>(
        getAccessorData("POSITION", [&](const auto& accessor) {
            vertexCount = accessor.count;
            min_ = glm::make_vec3(accessor.minValues.data());
            max_ = glm::make_vec3(accessor.maxValues.data());
    }));
    for (uint32_t i = 0; i < vertexCount; i++) {
        scene.vertices.push_back(glm::make_vec3(&positionPtr[i * 3]));
    }
    for (auto attr : attrs) {
        uint32_t count = 0;
        uint32_t stride = 0;
        auto* ptr = static_cast<const uint8_t*>(getAccessorData(attr, [&](auto& accessor) {
            count = accessor.count;
            switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_DOUBLE:
                    stride = sizeof(double);
                    break;
                case TINYGLTF_COMPONENT_TYPE_FLOAT:
                case TINYGLTF_COMPONENT_TYPE_INT:
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    stride = sizeof(uint32_t);
                    break;
                case TINYGLTF_COMPONENT_TYPE_SHORT:
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    stride = sizeof(uint16_t);
                    break;
                case TINYGLTF_COMPONENT_TYPE_BYTE:
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    stride = sizeof(uint8_t);
                    break;
            }
            switch (accessor.type) {
                case TINYGLTF_TYPE_VEC2: stride *= 2; break;
                case TINYGLTF_TYPE_VEC3: stride *= 3; break;
                case TINYGLTF_TYPE_VEC4: stride *= 4; break;
                case TINYGLTF_TYPE_MAT2: stride *= 4; break;
                case TINYGLTF_TYPE_MAT3: stride *= 9; break;
                case TINYGLTF_TYPE_MAT4: stride *= 16; break;
                default: WEN_WARN("unsupported accessor.type {} {} {}", accessor.type, __FILE__, __LINE__)
            }
        }));
        if (scene.attrDatas().find(attr) == scene.attrDatas().end()) {
            scene.attrDatas()[attr] = {};
        }
        WEN_DEBUG("{}: count {}, stride {}", attr, count, stride)
        auto& data = scene.attrDatas()[attr];
        auto start = data.size();
        data.resize(start + count * stride);
        memcpy(data.data() + start, ptr, count * stride);
    }

    const auto& indexAccessor = model.accessors[primitive.indices];
    const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
    indexCount = indexAccessor.count;
    auto* indexPtr = static_cast<const uint8_t*>(model.buffers[indexBufferView.buffer].data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset);
    switch (indexAccessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            for (uint32_t i = 0; i < indexCount; i++) {
                scene.indices.push_back(*(reinterpret_cast<const uint32_t*>(indexPtr)));
                indexPtr += 4;
            }
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            for (uint32_t i = 0; i < indexCount; i++) {
                scene.indices.push_back(*(reinterpret_cast<const uint16_t*>(indexPtr)));
                indexPtr += 2;
            }
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            for (uint32_t i = 0; i < indexCount; i++) {
                scene.indices.push_back(*(reinterpret_cast<const uint8_t*>(indexPtr)));
                indexPtr += 1;
            }
            break;
    }
}

GLTFPrimitive::~GLTFPrimitive() {}

} // namespace wen