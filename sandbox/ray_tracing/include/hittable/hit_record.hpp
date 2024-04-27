#pragma once

#include "resources/ray.hpp"

class Material;
class HitRecord {
public:
    float t;
    glm::vec3 point;
    bool inside;
    glm::vec3 normal;
    std::shared_ptr<Material> material;
    float u, v;

    void setNormal(const Ray& ray, const glm::vec3& outward) {
        inside = glm::dot(ray.direction, outward) < 0;
        normal = inside ? outward : -outward;
    }
};