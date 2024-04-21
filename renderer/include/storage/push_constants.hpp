#pragma once

#include "utils/enums.hpp"

namespace wen {

using ConstantType = VertexType;
struct PushConstantInfo {
    std::string name;
    ConstantType type;
};

class PushConstants final {
    friend class RenderPipeline;
    friend class Renderer;

public:
    PushConstants(ShaderStages stages, const std::vector<PushConstantInfo>& infos);
    ~PushConstants();

    void pushConstant(const std::string& name, const void* data);

private:
    uint32_t size_;
    std::vector<uint8_t> constants_;
    vk::PushConstantRange range_;
    std::map<std::string, uint32_t> offsets_;
    std::map<std::string, uint32_t> sizes_;
};

} // namespace wen