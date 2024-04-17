#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class Renderer;

class Framebuffer {
    friend class Renderer;

public:
    Framebuffer(const Renderer& renderer, uint32_t index);
    ~Framebuffer();

private:
    vk::Framebuffer framebuffer_;
};

class FramebufferStore {
    friend class Renderer;

public:
    FramebufferStore(const Renderer& renderer);
    ~FramebufferStore();

private:
    std::vector<std::unique_ptr<Framebuffer>> framebuffers_;
};

} // namespace wen