#include "ray_tracing/gltf/gltf_scene.hpp"
#include "storage/ktx_texture.hpp"
#include "storage/data_texture.hpp"
#include "core/logger.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace wen {

GLTFScene::GLTFScene(const std::string& filename, const std::vector<std::string>& attrs) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    size_t pos = filename.find_last_of('/');
    filepath_ = filename.substr(0, pos);

    auto filetype = filename.substr(filename.find_last_of('.') + 1);
    bool ret = false;
    if (filetype == "gltf") {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    } else if (filetype == "glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    } else {
        WEN_ERROR("unknown glTF filetype {}", filename)
    }

    if (!warn.empty()) {
        WEN_WARN("warn: {} {}", warn, filename);
    }
    if (!err.empty()) {
        WEN_ERROR("err: {} {}", err, filename);
    }
    if (!ret) {
        WEN_CRITICAL("failed to load glTF {}", filename);
        return;
    }

    for (auto& extension : model.extensionsRequired) {
        WEN_DEBUG("gltf model required \"{}\" extension", extension)
    }

    loadImages(model);
    loadMaterials(model);
    loadMeshesAndPrimitives(model, attrs);
    loadAttributes();
    loadNodes(model);
}

void GLTFScene::build(std::function<void(GLTFNode*, std::shared_ptr<GLTFPrimitive>)> fun) {
    for (auto* node : nodesPtr_) {
        if (node->getMesh().get() == nullptr) {
            continue;
        }
        for (auto& primitive : node->getMesh()->primitives) {
            fun(node, primitive);
        }
    }
}

void GLTFScene::loadImages(const tinygltf::Model& model) {
    for (auto& image : model.images) {
        if (image.uri.empty()) {
            WEN_WARN("unsupported image format {}", image.name)
            continue;
        }

        std::string::size_type pos;
        if ((pos = image.uri.find_last_of('.')) != std::string::npos) {
            if (image.uri.substr(pos + 1) == "ktx") {
                textures_.push_back(std::make_shared<KtxTexture>(filepath_ + "/" + image.uri));
                continue;
            }
        }

        std::vector<uint8_t> rgba;
        const uint8_t* data = nullptr;
        if (image.component == 3) {
            rgba.resize(image.width * image.height * 4);
            auto* rgb = image.image.data();
            auto* ptr = rgba.data();
            for (size_t i = 0; i < image.width * image.height; i++) {
                ptr[0] = rgb[0];
                ptr[1] = rgb[1];
                ptr[2] = rgb[2];
                ptr[3] = 0;
                ptr += 4;
                rgb += 3;
            }
            data = rgba.data();
            WEN_DEBUG("convert 3 channel(RGB) image to 4(RGBA) channel image {} X {}", image.width, image.height)
        } else {
            assert(image.component == 4);
            data = image.image.data();
            WEN_DEBUG("use 4 channel(RGBA) image {} X {}", image.width, image.height)
        }
        textures_.push_back(std::make_shared<DataTexture>(data, image.width, image.height, 0));
    }
    sampler_ = std::make_shared<Sampler>();
}

void GLTFScene::loadMaterials(const tinygltf::Model& model) {
    for (auto& material : model.materials) {
        auto& mat = materials_.emplace_back();
        mat.baseColorFactor = glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());
        mat.baseColorTexture = material.pbrMetallicRoughness.baseColorTexture.index;
        mat.emissiveFactor = glm::make_vec3(material.emissiveFactor.data());
        mat.emissiveTexture = material.emissiveTexture.index;
        mat.normalTexture = material.normalTexture.index;
        mat.metallicFactor = material.pbrMetallicRoughness.metallicFactor;
        mat.roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
        mat.metallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
    }
    materialBuffer_ = std::make_shared<StorageBuffer>(
        materials_.size() * sizeof(GLTFMaterial),
        vk::BufferUsageFlagBits::eStorageBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    auto* ptr = static_cast<uint8_t*>(materialBuffer_->map());
    memcpy(ptr, materials_.data(), materialBuffer_->getSize());
    materialBuffer_->unmap();
}

void GLTFScene::loadMeshesAndPrimitives(const tinygltf::Model& model, const std::vector<std::string>& attrs) {
    meshes_.push_back(std::make_shared<GLTFMesh>(*this, model, attrs));
}

void GLTFScene::loadAttributes() {
    for (auto& [name, data] : attrDatas_) {
        attrBuffers_[name] = std::make_shared<StorageBuffer>(
            data.size(),
            vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        );
        auto* ptr = static_cast<uint8_t*>(attrBuffers_[name]->map());
        memcpy(ptr, data.data(), attrBuffers_[name]->getSize());
        attrBuffers_[name]->unmap();
    } 
}

void GLTFScene::loadNodes(const tinygltf::Model& model) {
    auto defaultScene = model.scenes[std::max(0, model.defaultScene)];
    for (auto index : defaultScene.nodes) {
        auto node = std::make_shared<GLTFNode>(*this, model, index, nullptr);
        nodesPtr_.push_back(node.get());
        nodes_.push_back(std::move(node));
    }
}

GLTFScene::~GLTFScene() {
    nodes_.clear();
    nodesPtr_.clear();
    attrDatas_.clear();
    attrBuffers_.clear();
    meshes_.clear();
    materials_.clear();
    materialBuffer_.reset();
    textures_.clear();
    sampler_.reset();
    vertices.clear();
    indices.clear();
    rayTracingVertexBuffer.reset();
    rayTracingIndexBuffer.reset();
}

} // namespace wen