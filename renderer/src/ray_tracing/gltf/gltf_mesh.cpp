#include "ray_tracing/gltf/gltf_mesh.hpp"
#include "core/logger.hpp"

namespace wen {

GLTFMesh::GLTFMesh(GLTFScene& scene, const tinygltf::Model& model, const std::vector<std::string>& attrs) {
    for (auto& mesh : model.meshes) {
        WEN_DEBUG("Load mesh: {}", mesh.name)
        for (auto& primitive : mesh.primitives) {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                WEN_WARN("unsupported primitive mode: {}", primitive.mode)
                continue;
            }
            if (primitive.indices <= -1) {
                WEN_WARN("unsupported primitive indices: {}", primitive.indices)
                continue;
            }
            primitives.push_back(std::make_unique<GLTFPrimitive>(scene, model, primitive, attrs));
        }
    }
}

GLTFMesh::~GLTFMesh() {
    primitives.clear();
}

} // namespace wen