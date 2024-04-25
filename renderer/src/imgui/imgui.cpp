#include "imgui/imgui.hpp"
#include "core/setting.hpp"
#include "resources/render_pass.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace wen {

ImGuiLayer::ImGuiLayer(std::shared_ptr<Renderer>& renderer) : renderer_(renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize.x = static_cast<float>(settings->windowSize.width);
    io.DisplaySize.y = static_cast<float>(settings->windowSize.height);
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    ImGui::StyleColorsClassic();
    auto& style = ImGui::GetStyle();
    style.WindowMinSize = {160, 160};
    style.WindowRounding = 2;

    std::vector<vk::DescriptorPoolSize> poolSizes = {
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000}
    };
    vk::DescriptorPoolCreateInfo createInfo = {};
    createInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
        .setMaxSets(poolSizes.size() * 1000)
        .setPoolSizes(poolSizes);
    descriptorPool_ = manager->device->device.createDescriptorPool(createInfo);

    std::string src = renderer->renderPass->subpasses.back()->name;
    renderer->renderPass->addSubpass("imgui subpass")
        .setOutputAttachment(SWAPCHAIN_IMAGE_ATTACHMENT);
    renderer->renderPass->addSubpassDependency(
        src,
        "imgui subpass",
        {
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        },
        {
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentWrite
        }
    );
    renderer->updateRenderPass();

    std::vector<ImFontConfig> configs;
    if (settings->defaultFont.empty() && settings->chineseFont.empty()) {
        auto& config = configs.emplace_back();
        config.SizePixels = settings->fontSize;
        io.Fonts->AddFontDefault(&config);
    }
    if (!settings->defaultFont.empty()) {
        auto& config = configs.emplace_back();
        config.GlyphExtraSpacing = {0.25f, 0.0f};
        config.RasterizerMultiply = 1.2f;
        config.MergeMode = configs.size() != 1;
        io.Fonts->AddFontFromFileTTF(settings->defaultFont.c_str(), settings->fontSize, &config, io.Fonts->GetGlyphRangesDefault());
    }
    if (!settings->chineseFont.empty()) {
        auto& config = configs.emplace_back();
        config.GlyphExtraSpacing = {1.0f, 0.0f};
        config.RasterizerMultiply = 1.2f;
        config.MergeMode = configs.size() != 1;
        io.Fonts->AddFontFromFileTTF(settings->chineseFont.c_str(), settings->fontSize, &config, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    }
    io.Fonts->Build();

    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->getWindow());
    ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = manager->vkInstance;
    initInfo.PhysicalDevice = manager->device->physicalDevice;
    initInfo.Device = manager->device->device;
    initInfo.QueueFamily = manager->device->graphicsQueueFamilyIndex;
    initInfo.Queue = manager->device->graphicsQueue;
    initInfo.RenderPass = renderer->renderPass->renderPass;
    initInfo.PipelineCache = nullptr;
    initInfo.DescriptorPool = descriptorPool_;
    initInfo.Subpass = renderer->renderPass->subpasses.size() - 1;
    initInfo.MinImageCount = manager->swapchain->imageCount;
    initInfo.ImageCount = manager->swapchain->imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(settings->msaaSamples);
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = [](VkResult result) {
        if (result != VK_SUCCESS) {
            WEN_ERROR("ImGui Vulkan Error: {}", static_cast<uint32_t>(result))
        }
    };
    ImGui_ImplVulkan_Init(&initInfo);
}

void ImGuiLayer::begin() {
    renderer_->nextSubpass("imgui subpass");
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::end() {
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer_->getCurrentBuffer());
}

ImGuiLayer::~ImGuiLayer() {
    manager->device->device.waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    manager->device->device.destroyDescriptorPool(descriptorPool_);
    ImGui::DestroyContext();
}

} // namespace wen