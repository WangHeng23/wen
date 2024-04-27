#include "hittable/sphere.hpp"
#include <glm/ext/scalar_constants.hpp>

Sphere::Sphere(const glm::vec3& position, float radius, const std::shared_ptr<Material>& material) {
    moving = false;
    this->position = position;
    this->radius = radius;
    this->material = material;
    aabb = AABB(position - glm::vec3(radius), position + glm::vec3(radius));
}

Sphere::Sphere(const glm::vec3& src, const glm::vec3& dst, float radius, const std::shared_ptr<Material>& material) {
    moving = true;
    this->position = src;
    this->direction = dst - src;
    this->radius = radius;
    this->material = material;
    glm::vec3 rvec = glm::vec3(radius);
    AABB box1(src - rvec, src + rvec);
    AABB box2(dst - rvec, dst + rvec);
    aabb = AABB(box1, box2);
}

bool Sphere::hit(const Ray& ray, Interval t, HitRecord& hitRecord) const {
    glm::vec3 center = moving ? position + direction * ray.time : position;
    glm::vec3 origin = center - ray.origin;

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
    uv(normal, hitRecord.u, hitRecord.v);

    return true;
}

void Sphere::uv(const glm::vec3& point, float& u, float& v) {
    float theta = acos(-point.y);
    float phi = atan2(-point.z, point.x) + glm::pi<float>();

    u = phi / (2.0f * glm::pi<float>());
    v = theta / glm::pi<float>();
}