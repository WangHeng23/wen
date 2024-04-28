#pragma once

#include "hittable/hittable.hpp"

class Quad : public Hittable {
public:
    Quad(const glm::vec3& Q, const glm::vec3& u, const glm::vec3& v, const std::shared_ptr<Material>& material);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const override;

    glm::vec3 Q, u, v;
    std::shared_ptr<Material> material;
    glm::vec3 normal;
    float D;
    glm::vec3 w;

private:
    static bool isInterior(float a, float b, HitRecord& hitRecord);
};

inline std::shared_ptr<HittableList> box(const glm::vec3& a, const glm::vec3& b, const std::shared_ptr<Material>& material) {
    auto box = std::make_shared<HittableList>();

    auto min = glm::vec3(glm::min(a.x, b.x), glm::min(a.y, b.y), glm::min(a.z, b.z));
    auto max = glm::vec3(glm::max(a.x, b.x), glm::max(a.y, b.y), glm::max(a.z, b.z));

    auto dx = glm::vec3(max.x - min.x, 0, 0);
    auto dy = glm::vec3(0, max.y - min.y, 0);
    auto dz = glm::vec3(0, 0, max.z - min.z);

    box->add(std::make_shared<Quad>(glm::vec3(min.x, min.y, max.z),  dx,  dy, material)); // front
    box->add(std::make_shared<Quad>(glm::vec3(max.x, min.y, max.z), -dz,  dy, material)); // right
    box->add(std::make_shared<Quad>(glm::vec3(max.x, min.y, min.z), -dx,  dy, material)); // back
    box->add(std::make_shared<Quad>(glm::vec3(min.x, min.y, min.z),  dz,  dy, material)); // left
    box->add(std::make_shared<Quad>(glm::vec3(min.x, max.y, max.z),  dx, -dz, material)); // top
    box->add(std::make_shared<Quad>(glm::vec3(min.x, min.y, min.z),  dx,  dz, material)); // bottom

    return std::move(box);
}