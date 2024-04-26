#pragma once

#include "utils/enums.hpp"
#include "storage/uniform_buffer.hpp"
#include "resources/texture.hpp"
#include "resources/sampler.hpp"

namespace wen {

struct DescriptorInfo {
    DescriptorInfo(uint32_t binding, DescriptorType type, ShaderStages stage)
        : binding(binding), type(type), count(1), stage(stage), samplers(nullptr) {}
    
    DescriptorInfo(uint32_t binding, DescriptorType type, uint32_t count, ShaderStages stage)
        : binding(binding), type(type), count(count), stage(stage), samplers(nullptr) {}

    uint32_t binding;
    DescriptorType type;
    uint32_t count;
    ShaderStages stage;
    const vk::Sampler* samplers;
};

class Renderer;
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
    void bindTextures(uint32_t binding, const std::vector<std::pair<std::shared_ptr<Texture>, std::shared_ptr<Sampler>>>& textures_samplers);
    void bindTexture(uint32_t binding, std::shared_ptr<Texture> texture, std::shared_ptr<Sampler> sampler);
    void bindInputAttachments(uint32_t binding, const std::shared_ptr<Renderer>& renderer, const std::vector<std::pair<std::string, std::shared_ptr<Sampler>>>& names_samplers);
    void bindInputAttachment(uint32_t binding, const std::shared_ptr<Renderer>& renderer, const std::string& name, std::shared_ptr<Sampler> sampler);

private:
    const vk::DescriptorSetLayoutBinding& getLayoutBinding(uint32_t binding);

private:
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings_;
    vk::DescriptorSetLayout descriptorLayout_;
    std::vector<vk::DescriptorSet> descriptorSets_;
};

} // namespace wen