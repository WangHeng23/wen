#pragma once

#include "utils/enums.hpp"

namespace wen {

struct SamplerInfos {
    SamplerFilter magFilter = SamplerFilter::eLinear;
    SamplerFilter minFilter = SamplerFilter::eLinear;
    SamplerAddressMode addressModeU = SamplerAddressMode::eRepeat;
    SamplerAddressMode addressModeV = SamplerAddressMode::eRepeat;
    SamplerAddressMode addressModeW = SamplerAddressMode::eRepeat;
    uint32_t maxAnisotropy = 1;
    SamplerBorderColor borderColor = SamplerBorderColor::eIntOpaqueBlack;
    SamplerFilter mipmapMode = SamplerFilter::eLinear;
    uint32_t mipLevels = 1;
};

class Sampler final {
public:
    Sampler(const SamplerInfos& infos);
    ~Sampler();

    vk::Sampler sampler;
};

} // namespace wen