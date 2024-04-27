#include "ray_tracing.hpp"
#include "resources/material.hpp"
#include "hittable/sphere.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

void RandomSpheres(Scene& scene) {
    std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

    auto ground = std::make_shared<Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));
    world->add(std::make_shared<Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, ground));

    std::shared_ptr<Material> material;
    material = std::make_shared<Lambertian>(glm::vec3(0.4f, 0.2f, 0.1f));
    world->add(std::make_shared<Sphere>(glm::vec3(-3.0f, 1.0f, 0.0f), 1.0f, material));

    material = std::make_shared<Dielectric>(1.5);
    world->add(std::make_shared<Sphere>(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, material));

    material = std::make_shared<Metal>(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f);
    world->add(std::make_shared<Sphere>(glm::vec3(3.0f, 1.0f, 0.0f), 1.0f, material));

    int n = 4;
    for (int a = -n; a < n; a++) {
        for (int b = -n; b < n; b++) {
            auto type = Random::Float();
            glm::vec3 center(a + 0.9 * Random::Float(), 0.2, b + 0.9 * Random::Float());

            if ((center - glm::vec3(4, 0.2, 0)).length() > 0.9) {
                if (type < 0.8) {
                    if (type < 0.6f) {
                        material = std::make_shared<Lambertian>(Random::Vec3() * Random::Vec3());
                        world->add(std::make_shared<Sphere>(center, center + glm::vec3(0.0f, Random::Float(0.0f, 0.5f), 0.0f), 0.2f, material));
                    } else {
                        material = std::make_shared<Lambertian>(Random::Vec3());
                        world->add(std::make_shared<Sphere>(center, 0.2f, material));
                    }
                } else if (type < 0.9) {
                    auto albedo = Random::Vec3(0.5, 1);
                    auto roughness = Random::Float(0, 0.5);
                    material = std::make_shared<Metal>(albedo, roughness);
                    world->add(std::make_shared<Sphere>(center, 0.2, material));
                } else {
                    material = std::make_shared<Dielectric>(1.5);
                    world->add(std::make_shared<Sphere>(center, 0.2, material));
                }
            }
        }
    }

    scene.world = std::move(world);
}

RayTracing::RayTracing() : camera_(45.0f, 0.1f, 100.0f) {
    switch (1) {
        case 1: {
            RandomSpheres(scene_);
            auto direction = glm::normalize(glm::vec3(-13.0f, -2.0f, -3.0f));
            setCamera(glm::vec3(13.0f, 2.0f, 3.0f), direction);
            renderer_.background = glm::vec3(0.7f, 0.8f, 1.0f);
            break;
        }
    }
}

RayTracing::~RayTracing() {}

void RayTracing::update(float ts) {
    if (camera_.update(ts)) {
        renderer_.reset();
    }
}

void RayTracing::render() {
    ImGui::Begin("Scene");
    ImGui::SeparatorText("Camera");
    ImGui::DragFloat3("position", glm::value_ptr(camera_.position), 0.1f);
    ImGui::DragFloat3("direction", glm::value_ptr(camera_.direction), 0.1f);
    ImGui::Separator();
    ImGui::SeparatorText("Renderer");
    ImGui::ColorEdit3("background", glm::value_ptr(renderer_.background));
    ImGui::End();

    ImGui::Begin("Settings");
    ImGui::Text("fps: %.2f", ImGui::GetIO().Framerate);
    ImGui::Checkbox("accumulate", &renderer_.accumulated());
    ImGui::Text("frame index: %d", renderer_.index());
    if (ImGui::Button("reset frame index")) {
        renderer_.reset();
    }
    ImGui::Separator();
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("RayTracing");
    width_ = ImGui::GetContentRegionAvail().x;
    height_ = ImGui::GetContentRegionAvail().y;
    auto image = renderer_.image();
    if (image) {
        auto id = image->id();
        float w = image->width();
        float h = image->height();
        ImGui::Image(id, {w, h}, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
    ImGui::PopStyleVar();

    camera_.resize(width_, height_);
    renderer_.resize(width_, height_);
    renderer_.render(camera_, scene_);
}

void RayTracing::setCamera(const glm::vec3& position, const glm::vec3& direction) {
    camera_.position = position;
    camera_.direction = direction;
}