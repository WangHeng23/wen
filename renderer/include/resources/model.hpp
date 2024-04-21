#pragma once

#include "storage/vertex_buffer.hpp"
#include "storage/index_buffer.hpp"
#include <glm/glm.hpp>

namespace wen {

struct Vertex {
    glm::vec3 vertex;
    glm::vec3 normal;
    glm::vec3 color;

    bool operator==(const Vertex& other) const {
        return vertex == other.vertex && normal == other.normal && color == other.color;
    }
};

struct Offset {
    uint32_t vertex;
    uint32_t index;
};

class Mesh final {
public:
    Mesh();
    ~Mesh();

    Offset offset;
    std::vector<uint32_t> indices;
};

class Model {
public:
    Model(const std::string& filename);
    ~Model();

    uint32_t vertexCount;
    uint32_t indexCount;
    auto vertices() const { return vertices_; }
    auto meshes() const { return meshes_; }
    auto offset() const { return offset_; }

    Offset upload(std::shared_ptr<VertexBuffer> vertexBuffer,
                  std::shared_ptr<IndexBuffer> indexBuffer,
                  Offset offset = {0, 0});

private:
    std::vector<Vertex> vertices_;
    std::map<std::string, Mesh> meshes_;
    Offset offset_;
};

} // namespace wen