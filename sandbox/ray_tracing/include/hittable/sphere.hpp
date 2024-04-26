#pragma once

#include "hittable/hittable.hpp"

class Sphere : public Hittable {
public:
    Sphere(const glm::vec3& position, float radius, const std::shared_ptr<Material>& material);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const override;

    glm::vec3 position;
    float radius;
    std::shared_ptr<Material> material;
};