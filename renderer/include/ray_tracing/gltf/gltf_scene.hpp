#pragma once

#include "ray_tracing/ray_tracing_scene.hpp"
#include "ray_tracing/gltf/gltf_primitive.hpp"
#include "ray_tracing/gltf/gltf_mesh.hpp"
#include "ray_tracing/gltf/gltf_node.hpp"
#include "resources/texture.hpp"
#include "resources/sampler.hpp"
#include <tiny_gltf.h>

namespace wen {

struct GLTFMaterial {
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    int baseColorTexture = -1;
    glm::vec3 emissiveFactor = glm::vec3(0.0f);
    int emissiveTexture = -1;
    int normalTexture = -1;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    int metallicRoughnessTexture = -1;
};

class GLTFScene : public RayTracingScene {
public:
    GLTFScene(const std::string& filename, const std::vector<std::string>& attrs);
    ~GLTFScene() override;

    SceneType getType() const override {
        return SceneType::eGLTFScene;
    };

    void build(std::function<void(GLTFNode*, std::shared_ptr<GLTFPrimitive>)> func);

    auto& attrDatas() { return attrDatas_; }
    auto& nodesPtr() { return nodesPtr_; }
    auto getMeshes() { return meshes_; }
    auto getMaterialBuffer() { return materialBuffer_; }
    auto getAttrBuffer(const std::string& name) { return attrBuffers_.at(name); }
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    std::unique_ptr<Buffer> rayTracingVertexBuffer;
    std::unique_ptr<Buffer> rayTracingIndexBuffer;

private:
    void loadImages(const tinygltf::Model& model);
    void loadMaterials(const tinygltf::Model& model);
    void loadMeshesAndPrimitives(const tinygltf::Model& model, const std::vector<std::string>& attrs);
    void loadAttributes();
    void loadNodes(const tinygltf::Model& model);

private:
    std::string filepath_;

    // textures
    std::vector<std::shared_ptr<Texture>> textures_;
    std::shared_ptr<Sampler> sampler_;

    // materials
    std::vector<GLTFMaterial> materials_;
    std::shared_ptr<StorageBuffer> materialBuffer_;

    // meshes
    std::vector<std::shared_ptr<GLTFMesh>> meshes_;

    // attr
    std::map<std::string, std::vector<uint8_t>> attrDatas_;
    std::map<std::string, std::shared_ptr<StorageBuffer>> attrBuffers_;

    // nodes
    std::vector<std::shared_ptr<GLTFNode>> nodes_;
    std::vector<GLTFNode*> nodesPtr_;
};


} // namespace wen