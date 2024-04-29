#pragma once

#include "hittable/hittable.hpp"

class Sphere : public Hittable {
public:
    Sphere(const glm::vec3& position, float radius, const std::shared_ptr<Material>& material);
    Sphere(const glm::vec3& src, const glm::vec3& dst, float radius, const std::shared_ptr<Material>& material);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const override;
    float pdfValue(const glm::vec3& origin, const glm::vec3& direction) const override; 
    glm::vec3 random(const glm::vec3& origin) const override;

    bool moving;
    glm::vec3 position;
    glm::vec3 direction;
    float radius;
    std::shared_ptr<Material> material;

private:
    static void uv(const glm::vec3& point, float& u, float& v);
    static glm::vec3 random_(float radius, float distance2);
};