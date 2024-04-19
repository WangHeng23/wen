#pragma once

#include "utils/enums.hpp"
#include "storage/uniform_buffer.hpp"

namespace wen {

struct DescriptorInfo {
    DescriptorInfo(uint32_t binding, DescriptorType type, ShaderStages stage)
        : binding(binding), type(type), count(1), stage(stage), samplers(nullptr) {}

    uint32_t binding;
    DescriptorType type;
    uint32_t count;
    ShaderStages stage;
    const vk::Sampler* samplers;
};

class DescriptorSet final {
    friend class RenderPipeline;
    friend class Renderer;

public:
    DescriptorSet();
    ~DescriptorSet();

    DescriptorSet& addDescriptors(const std::vector<DescriptorInfo>& infos);
    void createDescriptorSetLayout();
    void allocateDescriptorSets();
    void freeDescriptorSets();
    void destroyDescriptorSetLayout();

    void build();

public:
    void bindUniforms(uint32_t binding, const std::vector<std::shared_ptr<UniformBuffer>>& uniforms);
    void bindUniform(uint32_t binding, std::shared_ptr<UniformBuffer> uniform);

private:
    const vk::DescriptorSetLayoutBinding& getLayoutBinding(uint32_t binding);

private:
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings_;
    vk::DescriptorSetLayout descriptorLayout_;
    std::vector<vk::DescriptorSet> descriptorSets_;
};

} // namespace wen