#pragma once

#include "hittable/hittable.hpp"

class Sphere : public Hittable {
public:
    Sphere(const glm::vec3& position, float radius, const std::shared_ptr<Material>& material);
    Sphere(const glm::vec3& src, const glm::vec3& dst, float radius, const std::shared_ptr<Material>& material);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const override;

    bool moving;
    glm::vec3 position;
    glm::vec3 direction;
    float radius;
    std::shared_ptr<Material> material;
};