#pragma once

#include "ray_tracing/gltf/gltf_mesh.hpp"
#include <glm/gtc/quaternion.hpp>

namespace wen {

class GLTFNode {
public:
    GLTFNode(GLTFScene& scene, const tinygltf::Model& model, uint32_t index, GLTFNode* parent);
    ~GLTFNode();

    auto getMesh() { return mesh_; }
    glm::mat4 getLocalMatrix();
    glm::mat4 getWorldMatrix();

private:
    std::string name_ = {};
    std::shared_ptr<GLTFMesh> mesh_ = {};

    GLTFNode* parent_ = {};
    std::vector<std::shared_ptr<GLTFNode>> children_ = {};

    glm::quat rotation_ = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale_ = glm::vec3(1.0f);
    glm::vec3 translation_ = glm::vec3(0.0f);
    glm::mat4 matrix_ = glm::mat4(1.0f);
};

} // namespace wen