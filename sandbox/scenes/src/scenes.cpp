#include "scenes.hpp"

void SceneManager::update() {
    scene_->update(ImGui::GetIO().DeltaTime);
}

void SceneManager::render() {
    scene_->renderer->acquireNextImage();
    if (!scene_->isEnableRayTracing) {
        scene_->renderer->beginRenderPass();
    }

    scene_->render();
    scene_->imguiLayer->begin();
    scene_->imgui();
    scene_->imguiLayer->end();

    scene_->renderer->endRenderPass();
    scene_->renderer->present();
}

SceneManager::~SceneManager() {
    scene_->renderer->waitIdle();
    scene_->destroy();
    scene_.reset();
    interface_.reset();
}