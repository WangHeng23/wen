#include "application.hpp"
#include "renderer.hpp"

class RayTracing : public Layer {
public:
    RayTracing();
    ~RayTracing();

    void update(float ts) override;
    void render() override;

private:
    Renderer renderer_;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
};