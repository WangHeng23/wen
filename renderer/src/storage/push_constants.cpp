#include "storage/push_constants.hpp"
#include "utils/utils.hpp"

namespace wen {

PushConstants::PushConstants(ShaderStages stages, const std::vector<PushConstantInfo>& infos) {
    size_ = 0;
    for (auto& info : infos) {
        uint32_t size = convert<uint32_t>(info.type);
        uint32_t align = 4;
        if (size > 4 && size <= 8) {
            align = 8;
        } else if (size > 8) {
            align = 16;
        }
        if (size_ % align != 0) {
            size_ += align - (size_ % align);
        }
        offsets_.insert(std::make_pair(info.name, size_));
        sizes_.insert(std::make_pair(info.name, size));
        size_ += size;
    }
    range_.setStageFlags(convert<vk::ShaderStageFlags>(stages))
          .setOffset(0)
          .setSize(size_);
    constants_.resize(size_);
}

PushConstants::~PushConstants() {
    sizes_.clear();
    offsets_.clear();
    constants_.clear();
}

void PushConstants::pushConstant(const std::string& name, const void* data) {
    memcpy(constants_.data() + offsets_.at(name), data, sizes_.at(name));
}

void PushConstants::pushConstant(const std::string& name, BasicValue value) {
    pushConstant(name, &value);
}

} // namespace wen