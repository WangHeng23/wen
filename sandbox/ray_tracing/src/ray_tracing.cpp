#include "ray_tracing.hpp"
#include "resources/material.hpp"
#include "hittable/sphere.hpp"
#include "hittable/bvh.hpp"
#include "hittable/quad.hpp"
#include "hittable/transform.hpp"
#include "hittable/constant_medium.hpp"
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

void CornellBox(Scene& scene) {
    std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

    auto red = std::make_shared<Lambertian>(glm::vec3(0.65f, 0.05f, 0.05f));
    auto white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
    auto green = std::make_shared<Lambertian>(glm::vec3(0.12f, 0.45f, 0.15f));
    auto light = std::make_shared<DiffuseLight>(glm::vec3(7.0f, 7.0f, 7.0f));

    world->add(std::make_shared<Quad>(
        glm::vec3(555.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 555.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 555.0f),
        green
    ));
    world->add(std::make_shared<Quad>(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 555.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 555.0f),
        red
    ));
    world->add(std::make_shared<Quad>(
        glm::vec3(113.0f, 554.0f, 127.0f),
        glm::vec3(330.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 305.0f),
        light
    ));
    world->add(std::make_shared<Quad>(
        glm::vec3(0.0f, 555.0f, 0.0f),
        glm::vec3(555.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 555.0f),
        white
    ));
    world->add(std::make_shared<Quad>(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(555.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 555.0f),
        white
    ));
    world->add(std::make_shared<Quad>(
        glm::vec3(0.0f, 0.0f, 555.0f),
        glm::vec3(555.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 555.0f, 0.0f),
        white
    ));

    std::shared_ptr<Hittable> box1 = box(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(165.0f, 330.0f, 165.0f),
        white
    );
    box1 = std::make_shared<Rotate>(box1, 15);
    box1 = std::make_shared<Translate>(box1, glm::vec3(265.0f, 0.0f, 295.0f));
    world->add(box1);
    // world->add(std::make_shared<ConstantMedium>(box1, 0.01f, glm::vec3(0.0f)));

    std::shared_ptr<Hittable> box2 = box(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(165.0f, 165.0f, 165.0f),
        white
    );
    box2 = std::make_shared<Rotate>(box2, -18.0f);
    box2 = std::make_shared<Translate>(box2, glm::vec3(130.0f, 0.0f, 65.0f));
    world->add(box2);
    // world->add(std::make_shared<ConstantMedium>(box2, 0.01f, glm::vec3(1.0f)));

    scene.world = std::move(world);
}

void FinalScene(Scene& scene) {
    std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

    // bottom
    std::shared_ptr<HittableList> bottom = std::make_shared<HittableList>();
    auto ground = std::make_shared<Lambertian>(glm::vec3(0.48f, 0.83f, 0.53f));
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            auto w = 100.0f;
            auto x0 = -1000.0f + i * w;
            auto z0 = -1000.0f + j * w;
            auto y0 = 0.0f;
            auto x1 = x0 + w;
            auto y1 = Random::Float(1.0f, 101.0f);
            auto z1 = z0 + w;
            bottom->add(box(glm::vec3(x0, y0, z0), glm::vec3(x1, y1, z1), ground));
        }
    }
    world->add(std::make_shared<BVH>(std::move(bottom)));

    // light
    world->add(std::make_shared<Quad>(
        glm::vec3(123.0f, 554.0f, 147.0f),
        glm::vec3(300.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 265.0f),
        std::make_shared<DiffuseLight>(glm::vec3(7.0f, 7.0f, 7.0f))
    ));

    // spheres
    std::shared_ptr<Material> material;
    material = std::make_shared<Dielectric>(1.5);
    world->add(std::make_shared<Sphere>(glm::vec3(260.0f, 150.0f, 45.0f), 50.0f, material));
    auto center1 = glm::vec3(400.0f, 400.0f, 200.0f);
    auto center2 = center1 + glm::vec3(30.0f, 0.0f, 0.0f);
    material = std::make_shared<Lambertian>(glm::vec3(0.7f, 0.3f, 0.1f));
    world->add(std::make_shared<Sphere>(center1, center2, 50.0f, material));
    material = std::make_shared<Metal>(glm::vec3(0.8f, 0.8f, 0.9f), 1.0f);
    world->add(std::make_shared<Sphere>(glm::vec3(0.0f, 150.0f, 145.0f), 50.0f, material));

    // boundary
    material = std::make_shared<Dielectric>(1.5);
    auto boundary1 = std::make_shared<Sphere>(glm::vec3(360.0f, 150.0f, 145.0f), 70.0f, material);
    world->add(boundary1);
    world->add(std::make_shared<ConstantMedium>(boundary1, 0.2f, glm::vec3(0.2f, 0.4f, 0.9f)));
    auto boundary2 = std::make_shared<Sphere>(glm::vec3(0.0f, 0.0f, 0.0f), 5000.0f, material);
    world->add(std::make_shared<ConstantMedium>(boundary2, 0.0001f, glm::vec3(1.0f, 1.0f, 1.0f)));

    // texture
    auto earthTexture = std::make_shared<ImageTexture>("sandbox/ray_tracing/resources/textures/earth.jpg");
    auto earthSurface = std::make_shared<Lambertian>(earthTexture);
    world->add(std::make_shared<Sphere>(glm::vec3(400.0f, 200.0f, 400.0f), 100.0f, earthSurface));
    auto perlinTexture = std::make_shared<NoiseTexture>(0.1f);
    auto perlinSurface = std::make_shared<Lambertian>(perlinTexture);
    world->add(std::make_shared<Sphere>(glm::vec3(220.0f, 280.0f, 300.0f), 80.0f, perlinSurface));

    // quad
    std::shared_ptr<HittableList> boxes = std::make_shared<HittableList>();
    auto white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
    for (int i = 0; i < 1000; i++) {
        boxes->add(std::make_shared<Sphere>(Random::Vec3(0.0f, 165.0f), 10.0f, white));
    }
    world->add(
        std::make_shared<Translate>(
            std::make_shared<Rotate>(
                std::make_shared<BVH>(boxes), 15.0f
            ),
            glm::vec3(-100.0f, 270.0f, 395.0f)
        )
    );

    scene.world = std::move(world);
}

RayTracing::RayTracing() : camera_(45.0f, 0.1f, 100.0f) {
    switch (3) {
        case 1: {
            RandomSpheres(scene_);
            auto direction = glm::normalize(glm::vec3(-13.0f, -2.0f, -3.0f));
            setCamera(glm::vec3(13.0f, 2.0f, 3.0f), direction);
            renderer_.samples = 4;
            renderer_.background = glm::vec3(0.7f, 0.8f, 1.0f);
            break;
        }
        case 2: {
            CornellBox(scene_);
            auto direction = glm::normalize(glm::vec3(0.0f, 0.0f, 800.0f));
            setCamera(glm::vec3(278.0f, 278.0f, -800.0f), direction);
            renderer_.samples = 4;
            break;
        }
        case 3: {
            FinalScene(scene_);
            auto direction = glm::normalize(glm::vec3(-200.0f, 0.0f, 600.0f));
            setCamera(glm::vec3(478.0f, 278.0f, -600.0f), direction);
            renderer_.samples = 4;
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
    ImGui::SliderInt("samples", &renderer_.samples, 1, 9);
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