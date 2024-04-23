#include "ray_tracing.hpp"
#include <imgui.h>

RayTracing::RayTracing() {}

RayTracing::~RayTracing() {}

void RayTracing::update(float ts) {}

void RayTracing::render() {
    ImGui::Begin("Scene");
    ImGui::End();

    ImGui::Begin("Settings");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("RayTracing");
    width_ = ImGui::GetContentRegionAvail().x;
    height_ = ImGui::GetContentRegionAvail().y;
    auto image = renderer_.getImage();
    if (image) {
        auto id = image->id();
        float w = image->width();
        float h = image->height();
        ImGui::Image(id, {w, h}, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
    ImGui::PopStyleVar();

    renderer_.resize(width_, height_);
    renderer_.render();
}