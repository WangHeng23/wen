#include "resources/shader.hpp"
#include "core/logger.hpp"
#include <shaderc/shaderc.hpp>
#include "manager.hpp"

namespace wen {

Shader::Shader(const std::vector<char>& codes) {
    vk::ShaderModuleCreateInfo info = {};
    info.setCodeSize(codes.size())
        .setPCode(reinterpret_cast<const uint32_t*>(codes.data()));
    module = manager->device->device.createShaderModule(info);
}

Shader::Shader(const std::string& filename, const std::string& code, ShaderStage stage) {
    shaderc_shader_kind kind;
    switch (stage) {
        case ShaderStage::eVertex: kind = shaderc_glsl_vertex_shader; break;
        case ShaderStage::eFragment: kind = shaderc_glsl_fragment_shader; break;
    }

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    auto result = compiler.CompileGlslToSpv(code, kind, filename.c_str(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        WEN_ERROR("Shader compilation failed: {}", result.GetErrorMessage());
        return;
    }

    std::vector<uint32_t> spirv(result.cbegin(), result.cend());
    vk::ShaderModuleCreateInfo info = {};
    info.setCodeSize(spirv.size() * sizeof(uint32_t))
        .setPCode(reinterpret_cast<const uint32_t*>(spirv.data()));
    module = manager->device->device.createShaderModule(info);
}

Shader::~Shader() {
    if (module.has_value()) {
        manager->device->device.destroyShaderModule(module.value());
        module.reset();
    }
}

} // namespace wen