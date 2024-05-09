#include "ray_tracing/gltf/gltf_node.hpp"
#include "ray_tracing/gltf/gltf_scene.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace wen {

GLTFNode::GLTFNode(GLTFScene& scene, const tinygltf::Model& model, uint32_t index, GLTFNode* parent) : parent_(parent) {
    auto& node = model.nodes[index];
    name_ = node.name;

    if (node.mesh > -1) {
        mesh_ = scene.getMeshes()[node.mesh];
    }
    if (node.rotation.size() == 4) {
        auto rotate = glm::make_quat(node.rotation.data());
        rotation_.x = rotate.w;
        rotation_.y = rotate.x;
        rotation_.z = rotate.y;
        rotation_.w = rotate.z;
    }
    if (node.scale.size() == 3) {
        scale_ = glm::make_vec3(node.scale.data());
    }
    if (node.translation.size() == 3) {
        translation_ = glm::make_vec3(node.translation.data());
    }
    if (node.matrix.size() == 16) {
        matrix_ = glm::make_mat4(node.matrix.data());
    }

    for (uint32_t idx : node.children) {
        auto node = std::make_shared<GLTFNode>(scene, model, idx, this);
        scene.nodesPtr().push_back(node.get());
        children_.push_back(std::move(node));
    }
}

glm::mat4 GLTFNode::getLocalMatrix() {
    return glm::translate(glm::mat4(1.0f), translation_) * glm::mat4(rotation_) * glm::scale(glm::mat4(1.0f), scale_) * matrix_;
}

glm::mat4 GLTFNode::getWorldMatrix() {
    if (parent_ != nullptr) {
        return parent_->getWorldMatrix() * getLocalMatrix();
    } else {
        return getLocalMatrix();
    }
}

GLTFNode::~GLTFNode() {
    children_.clear();
}

} // namespace wen