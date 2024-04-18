#include "resources/vertex_input.hpp"
#include "utils/utils.hpp"

namespace wen {

VertexInput::VertexInput(const std::vector<VertexInputInfo>& infos) {
    for (auto info : infos) {
        uint32_t offset = 0, size;
        for (auto format : info.formats) {
            attributeDescriptions_.push_back({
                location_,
                info.binding,
                convert<vk::Format>(format),
                offset
            });
            location_++;
            size = convert<uint32_t>(format);
            if (size > 16) {
                location_++;
            }
            offset += size;
        }
        bindingDescriptions_.push_back({
            info.binding,
            offset,
            convert<vk::VertexInputRate>(info.inputRate)
        });
    }
}

VertexInput::~VertexInput() {
    bindingDescriptions_.clear();
    attributeDescriptions_.clear();
}

} // namespace wen