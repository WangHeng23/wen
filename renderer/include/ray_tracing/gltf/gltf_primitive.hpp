#pragma once

#include "ray_tracing/ray_tracing_model.hpp"
#include <tiny_gltf.h>
#include <glm/glm.hpp>

namespace wen {

class GLTFScene;
class GLTFPrimitive : public RayTracingModel {
    friend class RayTracingInstance; // get scene_ to get ray_tracing (vertex/index)buffer

public:
    struct GLTFPrimitiveData {
        uint32_t firstIndex = 0;
        uint32_t firstVertex = 0;
        uint32_t materialIndex = 0;
    };

public:
    GLTFPrimitive(GLTFScene& scene, const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::vector<std::string>& attrs);
    ~GLTFPrimitive() override;

    ModelType getType() const override {
        return ModelType::eGLTFPrimitive;
    }

    auto& data() { return data_; }

public:
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

private:
    GLTFScene& scene_;
    GLTFPrimitiveData data_;
    glm::vec3 min_;
    glm::vec3 max_;
};

} // namespace wen