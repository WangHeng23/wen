#include "ray_tracing/render_pipeline.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

RayTracingRenderPipeline::RayTracingRenderPipeline(std::shared_ptr<RayTracingShaderProgram> shaderProgram) {
    shaderProgram_ = shaderProgram;
}

RayTracingRenderPipeline::~RayTracingRenderPipeline() {
    shaderProgram_.reset();
}

void RayTracingRenderPipeline::compile(const RayTracingRenderPipelineOptions& options) {
    std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;
    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    shaderGroups.reserve(1 + shaderProgram_->missShaders_.size() + shaderProgram_->hitGroups_.size());
    stages.reserve(1 + shaderProgram_->missShaders_.size() + shaderProgram_->hitShaderCount_);

    shaderGroups.emplace_back()
        .setType(vk::RayTracingShaderGroupTypeKHR::eGeneral)
        .setGeneralShader(stages.size());
    stages.push_back(
        createShaderStage(
            vk::ShaderStageFlagBits::eRaygenKHR,
            shaderProgram_->raygenShader_.shader->module.value(),
            shaderProgram_->raygenShader_.entry
        )
    );
    for (auto& miss : shaderProgram_->missShaders_) {
        shaderGroups.emplace_back()
            .setType(vk::RayTracingShaderGroupTypeKHR::eGeneral)
            .setGeneralShader(stages.size());
        stages.push_back(
            createShaderStage(
                vk::ShaderStageFlagBits::eMissKHR,
                miss.shader->module.value(),
                miss.entry
            )
        );
    }
    for (auto& hit : shaderProgram_->hitGroups_) {
        auto& shaderGroup = shaderGroups.emplace_back();
        shaderGroup.setClosestHitShader(stages.size());
        stages.push_back(
            createShaderStage(
                vk::ShaderStageFlagBits::eClosestHitKHR,
                hit.closestHitShader.shader->module.value(),
                hit.closestHitShader.entry
            )
        );
        if (hit.intersectionShader.has_value()) {
            shaderGroup.setType(vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup)
                       .setIntersectionShader(stages.size());
            stages.push_back(
                createShaderStage(
                    vk::ShaderStageFlagBits::eIntersectionKHR,
                    hit.intersectionShader.value().shader->module.value(),
                    hit.intersectionShader.value().entry
                )
            );
        } else {
            shaderGroup.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);
        }
    }
    
    createPipelineLayout();

    vk::RayTracingPipelineCreateInfoKHR info = {};
    info.setStages(stages)
        .setGroups(shaderGroups)
        .setMaxPipelineRayRecursionDepth(options.maxRayRecursionDepth)
        .setLayout(pipelineLayout);
    pipeline = manager->device->device.createRayTracingPipelineKHR({}, {}, info, nullptr, manager->dispatcher).value;

    vk::PhysicalDeviceProperties2 properties = {};
    vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
    properties.pNext = &rayTracingPipelineProperties;
    manager->device->physicalDevice.getProperties2(&properties);

    auto alignAddress = [](uint32_t size, uint32_t align) {
        return (size + (align - 1)) & ~(align - 1);
    };
    uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
    // 着色器绑定表 (缓存) 需要开头的组已经完成对齐并且组中的句柄也已经对齐完成
    uint32_t handleSizeAligned = alignAddress(handleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
    uint32_t baseAlignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
    raygenRegion_.setStride(alignAddress(handleSize, baseAlignment))
                 .setSize(raygenRegion_.stride);
    missRegion_.setStride(handleSizeAligned)
               .setSize(alignAddress(shaderProgram_->missShaders_.size() * handleSizeAligned, baseAlignment));
    hitRegion_.setStride(handleSizeAligned)
              .setSize(alignAddress(shaderProgram_->hitGroups_.size() * handleSizeAligned, baseAlignment));

    std::vector<uint8_t> handles(handleSize * shaderGroups.size());
    auto res = manager->device->device.getRayTracingShaderGroupHandlesKHR(pipeline, 0, shaderGroups.size(), handles.size(), handles.data(), manager->dispatcher);
    assert(res == vk::Result::eSuccess);
    // 分配用于存储着色器绑定表的缓存
    buffer_ = std::make_unique<Buffer>(
        raygenRegion_.size + missRegion_.size + hitRegion_.size,
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    // 获取每组的着色器绑定表
    auto address = getBufferAddress(buffer_->buffer);
    raygenRegion_.setDeviceAddress(address);
    missRegion_.setDeviceAddress(address + raygenRegion_.size);
    hitRegion_.setDeviceAddress(address + raygenRegion_.size + missRegion_.size);

    auto* ptr = static_cast<uint8_t*>(buffer_->map());
    auto getHandle = [&](int i) {
        return handles.data() + i * handleSize;
    };
    memcpy(ptr, getHandle(0), handleSize);
    ptr += raygenRegion_.size;
    for (uint32_t i = 0; i < shaderProgram_->missShaders_.size(); i ++) {
        memcpy(ptr + i * handleSizeAligned, getHandle(1 + i), handleSize);
    }
    ptr += missRegion_.size;
    for (uint32_t i = 0; i < shaderProgram_->hitGroups_.size(); i ++) {
        memcpy(ptr + i * handleSizeAligned, getHandle(1 + shaderProgram_->missShaders_.size() + i), handleSize);
    }
    buffer_->unmap();
}

} // namespace wen