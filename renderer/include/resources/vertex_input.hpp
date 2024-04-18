#pragma once

#include "utils/enums.hpp"

namespace wen {

struct VertexInputInfo {
    uint32_t binding;
    InputRate inputRate;
    std::vector<VertexType> formats;
};

class VertexInput final {
    friend class GraphicsRenderPipeline;

public:
    VertexInput(const std::vector<VertexInputInfo>& infos);
    ~VertexInput();

private:
    uint32_t location_ = 0;
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions_;
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions_;
};

} // namespace wen