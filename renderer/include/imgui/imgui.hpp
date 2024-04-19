#pragma once

#include "renderer.hpp"

namespace wen {

class ImGuiLayer final {
public:
    ImGuiLayer(std::shared_ptr<Renderer>& renderer);
    ~ImGuiLayer();

    void begin();
    void end();

private:
    std::shared_ptr<Renderer> renderer_;
    vk::DescriptorPool descriptorPool_;
};

} // namespace wen