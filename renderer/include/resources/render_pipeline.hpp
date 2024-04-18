#pragma once

#include "utils/enums.hpp"
#include "resources/shader_program.hpp"
#include "resources/vertex_input.hpp"
#include <vulkan/vulkan.hpp>

namespace wen {

class RenderPipeline {
public:
    RenderPipeline() = default;
    virtual ~RenderPipeline();

protected:
    void createPipelineLayout();

public:
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;
};

template <class RenderPipelineClass, typename Options>
class RenderPipelineTemplate : public RenderPipeline {
public:
    virtual ~RenderPipelineTemplate() = default;

    virtual void compile(const Options& options = {}) = 0;
};

class Renderer;
struct GraphicsRenderPipelineOptions {
    Topology topology = Topology::eTriangleList;
    PolygonMode polygonMode = PolygonMode::eFill;
    float lineWidth = 1.0f;
    CullMode cullMode = CullMode::eBack;
    FrontFace frontFace = FrontFace::eClockwise;
    std::vector<vk::DynamicState> dynamicStates;
};
class GraphicsRenderPipeline : public RenderPipelineTemplate<GraphicsRenderPipeline, GraphicsRenderPipelineOptions> {
public:
    GraphicsRenderPipeline(std::weak_ptr<Renderer> renderer, std::shared_ptr<GraphicsShaderProgram> shaderProgram, const std::string& subpassName);
    ~GraphicsRenderPipeline() override;

    void setVertexInput(std::shared_ptr<VertexInput> vertexInput);
    void compile(const GraphicsRenderPipelineOptions& options = {}) override;

private:
    std::weak_ptr<Renderer> renderer_;
    std::shared_ptr<GraphicsShaderProgram> shaderProgram_;
    std::string subpassName_;
    std::shared_ptr<VertexInput> vertexInput_;
};

} // namespace wen