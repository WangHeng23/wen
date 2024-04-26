#include "application.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

static Application* application = nullptr;

static vk::Instance gInstance = nullptr;
static vk::PhysicalDevice gPhysicalDevice = nullptr;
static vk::Device gDevice = nullptr;
static uint32_t gQueueFamily = 0;
static vk::Queue gQueue = nullptr;
static vk::PipelineCache gPipelineCache = nullptr;
static vk::DescriptorPool gDescriptorPool = nullptr;

static ImGui_ImplVulkanH_Window gMainWindowData;
static uint32_t gMinImageCount = 2;
static bool gSwapChainRebuild = false;

static uint32_t currentFrameIndex = 0;

static std::vector<std::vector<std::function<void()>>> sResourceFreeQueue;

static void frameRender(ImDrawData* data) {
    vk::Semaphore imageAcquiredSemaphore = gMainWindowData.FrameSemaphores[gMainWindowData.SemaphoreIndex].ImageAcquiredSemaphore;
    vk::Semaphore renderCompleteSemaphore = gMainWindowData.FrameSemaphores[gMainWindowData.SemaphoreIndex].RenderCompleteSemaphore;

    try {
        auto res = gDevice.acquireNextImageKHR(gMainWindowData.Swapchain, UINT64_MAX, imageAcquiredSemaphore, nullptr, &gMainWindowData.FrameIndex);
        if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) {
            gSwapChainRebuild = true;
            return;
        }
    } catch (vk::OutOfDateKHRError) {
        gSwapChainRebuild = true;
        return;
    }

    ImGui_ImplVulkanH_Frame* frame = &gMainWindowData.Frames[gMainWindowData.FrameIndex];
    auto result = gDevice.waitForFences({frame->Fence}, true, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        WEN_ERROR("Failed to wait for fence")
    }

    gDevice.resetFences({frame->Fence});

    currentFrameIndex = (currentFrameIndex + 1) % gMainWindowData.ImageCount;

    for (auto& func : sResourceFreeQueue[currentFrameIndex]) {
        func();
    }
    sResourceFreeQueue[currentFrameIndex].clear();

    gDevice.resetCommandPool(frame->CommandPool);

    vk::CommandBuffer commandBuffer = frame->CommandBuffer;
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);
    vk::RenderPassBeginInfo renderPassBegin = {};
    vk::ClearValue clearValues;
    clearValues.color.float32[0] = gMainWindowData.ClearValue.color.float32[0];
    clearValues.color.float32[1] = gMainWindowData.ClearValue.color.float32[1];
    clearValues.color.float32[2] = gMainWindowData.ClearValue.color.float32[2];
    clearValues.color.float32[3] = gMainWindowData.ClearValue.color.float32[3];
    renderPassBegin.setRenderPass(gMainWindowData.RenderPass)
        .setFramebuffer(frame->Framebuffer)
        .setRenderArea({{0, 0}, {(uint32_t)gMainWindowData.Width, (uint32_t)gMainWindowData.Height}})
        .setClearValueCount(1)
        .setClearValues(clearValues);
    commandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
    ImGui_ImplVulkan_RenderDrawData(data, frame->CommandBuffer);
    commandBuffer.endRenderPass();
    commandBuffer.end();

    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submits = {};
    submits.setWaitSemaphores(imageAcquiredSemaphore)
        .setWaitDstStageMask(waitStage)
        .setCommandBuffers(commandBuffer)
        .setSignalSemaphores(renderCompleteSemaphore);
    gQueue.submit(submits, frame->Fence);
}

static void framePresent() {
    if (gSwapChainRebuild) {
        return;
    }

    vk::Semaphore renderCompleteSemaphore = gMainWindowData.FrameSemaphores[gMainWindowData.SemaphoreIndex].RenderCompleteSemaphore;
    vk::PresentInfoKHR presentInfo = {};
    vk::SwapchainKHR swapchain = gMainWindowData.Swapchain;
    presentInfo.setWaitSemaphores(renderCompleteSemaphore)
        .setSwapchains(swapchain)
        .setImageIndices(gMainWindowData.FrameIndex)
        .setPResults(nullptr);
    
    try {
        auto res = gQueue.presentKHR(presentInfo); 
        if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) {
            gSwapChainRebuild = true;
            return;
        }
    } catch (vk::OutOfDateKHRError) {
        gSwapChainRebuild = true;
        return;
    }

    gMainWindowData.SemaphoreIndex = (gMainWindowData.SemaphoreIndex + 1) % gMainWindowData.ImageCount;
}

Application::Application() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(
        static_cast<int>(wen::settings->windowInfo.width),
        static_cast<int>(wen::settings->windowInfo.height),
        wen::settings->appName.c_str(),
        nullptr, nullptr
    );
}

void Application::init() {
    // Instance
    {
        uint32_t extensionsCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        vk::InstanceCreateInfo createInfo = {};
        const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
        createInfo.setEnabledExtensionCount(extensionsCount)
            .setPpEnabledExtensionNames(extensions)
            .setEnabledLayerCount(1)
            .setPEnabledLayerNames(layers);
        gInstance = vk::createInstance(createInfo);
    }
    // Device
    {
        gPhysicalDevice = gInstance.enumeratePhysicalDevices().front();

        auto queueFamilies = gPhysicalDevice.getQueueFamilyProperties();
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                gQueueFamily = i;
                break;
            }
        }
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.setQueueFamilyIndex(gQueueFamily)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);
        const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
        vk::DeviceCreateInfo createInfo = {};
        createInfo.setQueueCreateInfos(queueCreateInfo)
            .setPEnabledExtensionNames(extensions)
            .setPEnabledLayerNames(layers);
        gDevice = gPhysicalDevice.createDevice(createInfo);
        gQueue = gDevice.getQueue(gQueueFamily, 0);
    }
    // DescriptorPool
    {
        vk::DescriptorPoolSize sizes[] = {
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
            .setMaxSets(1000 * IM_ARRAYSIZE(sizes))
            .setPoolSizeCount(static_cast<uint32_t>(IM_ARRAYSIZE(sizes)))
            .setPoolSizes(sizes);
        gDescriptorPool = gDevice.createDescriptorPool(createInfo);
    }
    // Surface
    {
        glfwCreateWindowSurface(gInstance, window_, nullptr, &gMainWindowData.Surface);
    }
    // Swapchain, CommandPool, CommandBuffer
    {
        const vk::Format formats[] = {
            vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm,
            vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm
        };
        gMainWindowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
            gPhysicalDevice,
            gMainWindowData.Surface,
            (VkFormat*)formats,
            4,
            (VkColorSpaceKHR)(vk::ColorSpaceKHR::eSrgbNonlinear)
        );
        VkPresentModeKHR presentModes[] = {VK_PRESENT_MODE_FIFO_KHR};
        gMainWindowData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
            gPhysicalDevice,
            gMainWindowData.Surface,
            &presentModes[0],
            1
        );
        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);
        ImGui_ImplVulkanH_CreateOrResizeWindow(
            gInstance,
            gPhysicalDevice,
            gDevice,
            &gMainWindowData,
            gQueueFamily,
            nullptr,
            width, height,
            gMinImageCount
        );
    }
    sResourceFreeQueue.resize(gMainWindowData.ImageCount);
    // Setup ImGui Context
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsClassic();
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplGlfw_InitForVulkan(window_, true);
        ImGui_ImplVulkan_InitInfo info = {};
        info.Instance = gInstance;
        info.PhysicalDevice = gPhysicalDevice;
        info.Device = gDevice;
        info.QueueFamily = gQueueFamily;
        info.Queue = gQueue;
        info.RenderPass = gMainWindowData.RenderPass;
        info.PipelineCache = gPipelineCache;
        info.DescriptorPool = gDescriptorPool;
        info.Subpass = 0;
        info.MinImageCount = gMinImageCount;
        info.ImageCount = gMainWindowData.ImageCount;
        info.MSAASamples = (VkSampleCountFlagBits)vk::SampleCountFlagBits::e1;
        info.Allocator = nullptr;
        info.CheckVkResultFn = [](VkResult res) {
            if (res != VK_SUCCESS) {
                fprintf(stderr, "[vulkan] Error: VkResult = %d\n", res);
            }
        };
        ImGui_ImplVulkan_Init(&info);
    }
    // Load ImGui Font
    {
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;
        io.FontDefault = io.Fonts->AddFontFromFileTTF(wen::settings->defaultFont.c_str(), 23, &fontConfig);

        vk::CommandPool commandPool = gMainWindowData.Frames[gMainWindowData.FrameIndex].CommandPool;
        vk::CommandBuffer commandBuffer = gMainWindowData.Frames[gMainWindowData.FrameIndex].CommandBuffer;
        gDevice.resetCommandPool(commandPool);
        vk::CommandBufferBeginInfo beginInfo = {};
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);
        ImGui_ImplVulkan_CreateFontsTexture();
        commandBuffer.end();
        vk::SubmitInfo submits = {};
        submits.setCommandBuffers(commandBuffer);
        gQueue.submit(submits, nullptr);
        gDevice.waitIdle();
        ImGui_ImplVulkan_DestroyFontsTexture();
    }
}

void Application::run() {
    application = this;
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        for (auto& layer : layers_) {
            layer->update(ImGui::GetIO().DeltaTime);
        }
        if (gSwapChainRebuild) {
            int width, height;
            glfwGetFramebufferSize(window_, &width, &height);
            if (width > 0 && height > 0) {
                ImGui_ImplVulkan_SetMinImageCount(gMinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(
                    gInstance,
                    gPhysicalDevice,
                    gDevice,
                    &gMainWindowData,
                    gQueueFamily,
                    nullptr,
                    width, height,
                    gMinImageCount
                );
                gMainWindowData.FrameIndex = 0;
                gSwapChainRebuild = false;
            }
        }
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
            window_flags |= ImGuiWindowFlags_NoBackground;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        for (auto& layer : layers_) {
            layer->render();
        }
        ImGui::End();
        ImGui::Render();
        ImDrawData* data = ImGui::GetDrawData();
        const bool minimized = (data->DisplaySize.x <= 0.0f || data->DisplaySize.y <= 0.0f);
        ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        gMainWindowData.ClearValue.color.float32[0] = clearColor.x * clearColor.w;
        gMainWindowData.ClearValue.color.float32[1] = clearColor.y * clearColor.w;
        gMainWindowData.ClearValue.color.float32[2] = clearColor.z * clearColor.w;
        gMainWindowData.ClearValue.color.float32[3] = clearColor.w;

        if (!minimized) {
            frameRender(data);
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        if (!minimized) {
            framePresent();
        }
    }
}

Application::~Application() {
    application = nullptr;
    layers_.clear();

    gDevice.waitIdle();
    for (auto& queue : sResourceFreeQueue) {
        for (auto& func : queue) {
            func();
        }
    }
    sResourceFreeQueue.clear();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImGui_ImplVulkanH_DestroyWindow(gInstance, gDevice, &gMainWindowData, nullptr);

    gDevice.destroyDescriptorPool(gDescriptorPool);
    gDevice.destroy();
    gInstance.destroy();
}

Application& Application::get() {
    return *application;
}

vk::Instance Application::getInstance() {
    return gInstance;
}

vk::PhysicalDevice Application::getPhysicalDevice() {
    return gPhysicalDevice;
}

vk::Device Application::getDevice() {
    return gDevice;
}

vk::CommandBuffer Application::allocateSingleUse() {
    ImGui_ImplVulkanH_Window* wd = &gMainWindowData;
    vk::CommandPool commandPool = wd->Frames[wd->FrameIndex].CommandPool;

    vk::CommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(1);

    auto cmdbuf = gDevice.allocateCommandBuffers(allocateInfo)[0];
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdbuf.begin(beginInfo);

    return cmdbuf;
}

void Application::freeSingleUse(vk::CommandBuffer cmdbuf) {
    cmdbuf.end();

    ImGui_ImplVulkanH_Window* wd = &gMainWindowData;
    vk::CommandPool commandPool = wd->Frames[wd->FrameIndex].CommandPool;

    vk::SubmitInfo submits = {};
    submits.setCommandBuffers(cmdbuf);
    gQueue.submit(submits, nullptr);
    gQueue.waitIdle();
    gDevice.freeCommandBuffers(commandPool, cmdbuf);
}

void Application::submitResourceFree(std::function<void()>&& func) {
    sResourceFreeQueue[currentFrameIndex].emplace_back(func);
}