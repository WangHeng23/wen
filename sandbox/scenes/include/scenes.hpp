#pragma once

#include <wen.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

class Scene {
    friend class SceneManager;

public:
    Scene(std::shared_ptr<wen::Interface> interface) : interface(interface) {}
    virtual ~Scene() = default;

    virtual void initialize() = 0;
    virtual void update(float ts) = 0;
    virtual void render() = 0;
    virtual void imgui() = 0;
    virtual void destroy() = 0;

protected:
    bool isEnableRayTracing = false;
    std::shared_ptr<wen::Interface> interface;
    std::shared_ptr<wen::Renderer> renderer;
    std::shared_ptr<wen::ImGuiLayer> imguiLayer;
};

class SceneManager {
public:
    SceneManager(std::shared_ptr<wen::Interface> interface) : interface_(interface) {}
    ~SceneManager();

    template <class Scene>
    void setScene() {
        scene_ = std::make_unique<Scene>(interface_);
        scene_->initialize();
    }

    void update();
    void render();

private:
    std::shared_ptr<wen::Interface> interface_;
    std::unique_ptr<Scene> scene_;
};