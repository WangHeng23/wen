#include "ray_tracing.hpp"
#include "resources/material.hpp"
#include "hittable/sphere.hpp"
#include "hittable/bvh.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void RandomSpheres(Scene& scene) {
    std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

    auto ground = std::make_shared<Lambertian>(
        std::make_shared<ChessboardTexture>(0.32f, glm::vec3(0.2f, 0.3f, 0.1f), glm::vec3(0.9f, 0.9f, 0.9f))
    );
    world->add(std::make_shared<Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, ground));

    std::shared_ptr<Material> material;
    material = std::make_shared<Lambertian>(glm::vec3(0.4f, 0.2f, 0.1f));
    world->add(std::make_shared<Sphere>(glm::vec3(-3.0f, 1.0f, 0.0f), 1.0f, material));

    material = std::make_shared<Dielectric>(1.5);
    world->add(std::make_shared<Sphere>(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, material));

    material = std::make_shared<Metal>(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f);
    world->add(std::make_shared<Sphere>(glm::vec3(3.0f, 1.0f, 0.0f), 1.0f, material));

    auto texture = std::make_shared<ChessboardTexture>(0.2f, glm::vec3(0.2f, 0.3f, 0.1f), glm::vec3(0.9f, 0.9f, 0.9f));
    material = std::make_shared<Lambertian>(texture);
    world->add(std::make_shared<Sphere>(glm::vec3(10.0f, 1.0f, 0.0f), 1.0f, material));

    auto earth = std::make_shared<ImageTexture>("sandbox/ray_tracing/resources/textures/earth.jpg");
    material = std::make_shared<Lambertian>(earth);
    world->add(std::make_shared<Sphere>(glm::vec3(7.0f, 1.0f, 0.0f), 1.0f, material));

    auto pertex = std::make_shared<NoiseTexture>(4.0f);
    material = std::make_shared<Lambertian>(pertex);
    world->add(std::make_shared<Sphere>(glm::vec3(3.0f, 1.0f, 2.5f), 1.0f, material));

    int n = 8;
    for (int a = -n; a < n; a++) {
        for (int b = -n; b < n; b++) {
            auto type = Random::Float();
            glm::vec3 center(a + 0.9f * Random::Float(), 0.2f, b + 0.9f * Random::Float());

            if ((center - glm::vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f) {
                if (type < 0.8f) {
                    if (type < 0.6f) {
                        material = std::make_shared<Lambertian>(Random::Vec3() * Random::Vec3());
                        world->add(std::make_shared<Sphere>(center, center + glm::vec3(0.0f, Random::Float(0.0f, 0.5f), 0.0f), 0.2f, material));
                    } else {
                        material = std::make_shared<Lambertian>(Random::Vec3());
                        world->add(std::make_shared<Sphere>(center, 0.2f, material));
                    }
                } else if (type < 0.9f) {
                    auto albedo = Random::Vec3(0.5f, 1.0f);
                    auto roughness = Random::Float(0.0f, 0.5f);
                    material = std::make_shared<Metal>(albedo, roughness);
                    world->add(std::make_shared<Sphere>(center, 0.2f, material));
                } else {
                    material = std::make_shared<Dielectric>(1.5f);
                    world->add(std::make_shared<Sphere>(center, 0.2f, material));
                }
            }
        }
    }

    // scene.world = std::move(world);
    scene.world = std::move(std::make_shared<HittableList>(std::move(std::make_shared<BVH>(world))));
}

RayTracing::RayTracing() : camera_(45.0f, 0.1f, 100.0f) {
    switch (1) {
        case 1: {
            RandomSpheres(scene_);
            auto direction = glm::normalize(glm::vec3(-13.0f, -2.0f, -3.0f));
            setCamera(glm::vec3(13.0f, 2.0f, 3.0f), direction);
            renderer_.samples = 4;
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
    ImGui::SliderInt("samples", &renderer_.samples, 1, 8);
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
    ImGui::InputText("filename", filename_, 1024);
    if (ImGui::Button("save image")) {
        auto image = renderer_.image();
        auto data = renderer_.data();
        auto filename = "sandbox/ray_tracing/resources/images/" + std::string(filename_);
        stbi_flip_vertically_on_write(true);
        stbi_write_png(filename.c_str(), image->width(), image->height(), 4, data, 0);
    }
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