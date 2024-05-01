#pragma once

#include <shaderc/shaderc.hpp>

namespace wen {

class ShaderIncluder final : public shaderc::CompileOptions::IncluderInterface {
public:
    ShaderIncluder(const std::string& filepath);
    shaderc_include_result* GetInclude(
        const char* requested_source,
        shaderc_include_type type,
        const char* requesting_source,
        size_t include_depth
    ) override;
    void ReleaseInclude(shaderc_include_result* data) override;

private:
    std::string filepath_;
};

} // namespace wen