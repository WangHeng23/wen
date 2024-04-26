#include "hittable/sphere.hpp"

Sphere::Sphere(const glm::vec3& position, float radius, const std::shared_ptr<Material>& material) {
    this->position = position;
    this->radius = radius;
    this->material = material;
}

bool Sphere::hit(const Ray& ray, Interval t, HitRecord& hitRecord) const {
    glm::vec3 origin = position - ray.origin;

    float a = glm::dot(ray.direction, ray.direction);
    float h = glm::dot(ray.direction, origin);
    float c = glm::dot(origin, origin) - radius * radius;

    float discriminant = h * h - a * c;
    if (discriminant < 0.0f) {
        return false;
    }

    float sqtrd = glm::sqrt(discriminant);
    float root = (h - sqtrd) / a;
    if (!t.inside(root)) {
        root = (h + sqtrd) / a;
        if (!t.inside(root)) {
            return false;
        }
    }

    hitRecord.t = root;
    hitRecord.point = ray.hitPoint(root);
    glm::vec3 normal = (hitRecord.point - position) / radius;
    hitRecord.setNormal(ray, normal);
    hitRecord.material = material;

    return true;
}