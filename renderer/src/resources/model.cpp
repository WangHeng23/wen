#include "resources/model.hpp"
#include "core/logger.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {

template <>
struct hash<wen::Vertex> {
    size_t operator()(wen::Vertex const& vertex) const {
        return ((hash<glm::vec3>()(vertex.position) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.color) << 1);
    }
};

} // namespace std

namespace wen {

Mesh::Mesh() {

}

Mesh::~Mesh() {
    indices.clear();
}

Model::Model(const std::string& filename) : vertexCount(0), indexCount(0) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
        WEN_ERROR("Failed to load model: {0}", filename);
        throw std::runtime_error(warn + err);
    }

    uint32_t size = 0;
    for (const auto& shape : shapes) {
        size += shape.mesh.indices.size();
    }
    vertices_.reserve(size);

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
    uniqueVertices.reserve(size);

    for (const auto& shape : shapes) {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            if (index.normal_index < 0) {
                vertex.normal = {0.0f, 0.0f, 0.0f};
            } else {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices.insert(std::make_pair(vertex, vertices_.size()));
                vertices_.push_back(vertex);
            }
            mesh->indices.push_back(uniqueVertices[vertex]);
        }
        indexCount += mesh->indices.size();
        meshes_.insert(std::make_pair(shape.name, std::move(mesh)));
    }
    vertexCount = vertices_.size();
}

Offset Model::upload(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, Offset offset) {
    offset_ = offset;
    auto temp = offset;
    temp.vertex = vertexBuffer->upload(vertices_, temp.vertex);
    for (auto& [name, mesh] : meshes_) {
        mesh->offset.vertex = offset_.vertex;
        mesh->offset.index = temp.index;
        temp.index = indexBuffer->upload(mesh->indices, temp.index);
    }
    return temp;
}

Model::~Model() {
    vertices_.clear();
    meshes_.clear();
    rayTracingVertexBuffer.reset();
    rayTracingIndexBuffer.reset();
}

} // namespace wen