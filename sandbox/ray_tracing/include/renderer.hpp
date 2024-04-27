#pragma once

#include "image.hpp"
#include "camera.hpp"
#include "hittable/hittable.hpp"

struct Scene {
    std::shared_ptr<HittableList> world;
};

class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;

    void resize(uint32_t width, uint32_t height);
    void render(const Camera& camera, const Scene& scene);

    std::shared_ptr<Image> image() const { return image_; }
    uint32_t* data() const { return data_; }

    int index() const { return index_; }
    bool& accumulated() { return accumulated_; }
    void reset() { index_ = 1; }

public: 
    int samples = 1;
    glm::vec3 background = glm::vec3(0.0f);

private:
    Ray pixel(uint32_t x, uint32_t y, int s);
    glm::vec3 traceRay(const Ray& ray, int depth);

private:
    const Camera* camera_;
    const Scene* scene_;

    std::shared_ptr<Image> image_ = nullptr;
    uint32_t* data_ = nullptr;

    glm::vec4* accumulation_ = nullptr;
    bool accumulated_ = true;
    uint32_t index_ = 1;

    std::vector<uint32_t> horizontal_;
    std::vector<uint32_t> vertical_;
};