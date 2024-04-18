#pragma once

#include "utils/enums.hpp"
#include <vulkan/vulkan.hpp>

namespace wen {

class Shader final {
public:
    Shader(const std::vector<char>& codes);
    Shader(const std::string& filename, const std::string& code, ShaderStage stage);
    ~Shader();

    std::optional<vk::ShaderModule> module;
};

} // namespace wen