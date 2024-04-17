#include "resources/shader.hpp"
#include "manager.hpp"

namespace wen {

Shader::Shader(const std::vector<char>& codes) {
    vk::ShaderModuleCreateInfo info = {};
    info.setCodeSize(codes.size())
        .setPCode(reinterpret_cast<const uint32_t*>(codes.data()));
    module = manager->device->device.createShaderModule(info);
}

Shader::~Shader() {
    if (module.has_value()) {
        manager->device->device.destroyShaderModule(module.value());
        module.reset();
    }
}

} // namespace wen