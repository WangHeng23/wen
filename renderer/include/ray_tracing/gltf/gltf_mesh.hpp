#pragma once

#include "ray_tracing/gltf/gltf_primitive.hpp"

namespace wen {

class GLTFMesh {
public:
    GLTFMesh(GLTFScene& scene, const tinygltf::Model& model, const std::vector<std::string>& attrs);
    ~GLTFMesh();

    std::vector<std::shared_ptr<GLTFPrimitive>> primitives = {};
};

} // namespace wen