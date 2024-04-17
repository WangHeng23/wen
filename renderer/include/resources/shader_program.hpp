#pragma once

#include "shader.hpp"

namespace wen {

template <class ShaderProgramClass>
struct ShaderProgramBindPoint {
    const static vk::PipelineBindPoint bindPoint;
};

class ShaderProgram {
public:
    struct ShaderStage {
        ShaderStage() : shader(nullptr), entry("") {}
        ShaderStage(std::shared_ptr<Shader> shader) : shader(shader), entry("main") {}
        ShaderStage(std::shared_ptr<Shader> shader, const std::string& entry) : shader(shader), entry(entry) {}

        std::shared_ptr<Shader> shader;
        std::string entry;
    };

public:
    ShaderProgram() = default;
    virtual ~ShaderProgram() = default;
};

class GraphicsShaderProgram : public ShaderProgram {
    friend class GraphicsRenderPipeline;

public:
    GraphicsShaderProgram() = default;
    ~GraphicsShaderProgram() override;

public:
    void setVertexShader(std::shared_ptr<Shader> shader, const std::string& entry = "main");
    void setFragmentShader(std::shared_ptr<Shader> shader, const std::string& entry = "main");

private:
    ShaderStage vertexShader_;
    ShaderStage fragmentShader_;
};

template <>
struct ShaderProgramBindPoint<GraphicsShaderProgram> {
    const static vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
};

} // namespace wen