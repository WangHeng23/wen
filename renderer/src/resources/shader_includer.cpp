#include "resources/shader_includer.hpp"
#include "utils/utils.hpp"

namespace wen {

ShaderIncluder::ShaderIncluder(const std::string& filepath) {
    if (filepath.find('/') == std::string::npos) {
        filepath_ = "./";
    } else {
        filepath_ = filepath.substr(0, filepath.find_last_of('/') + 1);
    }
}

shaderc_include_result* ShaderIncluder::GetInclude(
    const char* requested_source,
    shaderc_include_type type,
    const char* requesting_source,
    size_t include_depth
) {
    auto result = new shaderc_include_result();
    std::string* contents;
    if (type == shaderc_include_type_standard) {
        if (strcmp(requested_source, "RayTracing") == 0) {
            result->source_name = "RayTracing";
            contents = new std::string(
                "#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n"
                "\n"
                "struct InstanceAddressInfo {\n"
                "    uint64_t vertexBufferAddress;\n"
                "    uint64_t indexBufferAddress;\n"
                "};\n"
                "\n"
                "struct Vertex {\n"
                "    vec3 position;\n"
                "    vec3 normal;\n"
                "    vec3 color;\n"
                "};\n"
                "\n"
                "struct Index {\n"
                "    uint i0; \n"
                "    uint i1; \n"
                "    uint i2; \n"
                "};\n"
            );
        }
    } else {
        result->source_name = requested_source;
        auto data = readFile(filepath_ + std::string(requested_source));
        contents = new std::string(data.begin(), data.end());
    }

    result->source_name_length = strlen(result->source_name);
    result->content = contents->c_str();
    result->content_length = contents->size();
    result->user_data = contents;

    return result;
}

void ShaderIncluder::ReleaseInclude(shaderc_include_result* data) {
    delete static_cast<std::string*>(data->user_data);
    delete data;
}

} // namespace wen