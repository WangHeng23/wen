#pragma once

#include "utils/enums.hpp"

namespace wen {

using ConstantType = VertexType;
struct PushConstantInfo {
    std::string name;
    ConstantType type;
};

union BasicValue {
    bool b;
    BasicValue(bool b) : b(b) {}
    int i;
    BasicValue(int i) : i(i) {}
    uint32_t u;
    BasicValue(uint32_t u) : u(u) {}
    float f;
    BasicValue(float f) : f(f) {}
};

class PushConstants final {
    friend class RenderPipeline;
    friend class Renderer;

public:
    PushConstants(ShaderStages stages, const std::vector<PushConstantInfo>& infos);
    ~PushConstants();

    void pushConstant(const std::string& name, const void* data);
    void pushConstant(const std::string& name, BasicValue value);

private:
    uint32_t size_;
    std::vector<uint8_t> constants_;
    vk::PushConstantRange range_;
    std::map<std::string, uint32_t> offsets_;
    std::map<std::string, uint32_t> sizes_;
};

} // namespace wen