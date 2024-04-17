#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class Shader final {
public:
    Shader(const std::vector<char>& codes);
    ~Shader();

    std::optional<vk::ShaderModule> module;
};

} // namespace wen