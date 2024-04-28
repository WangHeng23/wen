#include "hittable/transform.hpp"

Translate::Translate(const std::shared_ptr<Hittable>& hittable, const glm::vec3& displacement)
    : hittable_(hittable), offset_(displacement) {
    aabb = hittable->aabb + offset_;
}

bool Translate::hit(const Ray& ray, Interval t, HitRecord& hitRecord) const {
    Ray rayOffset(ray.origin - offset_, ray.direction, ray.time);

    if (!hittable_->hit(rayOffset, t, hitRecord)) {
        return false;
    }

    hitRecord.point += offset_;

    return true;
}

Rotate::Rotate(const std::shared_ptr<Hittable>& hittable, float angle) : hittable_(hittable) {
    auto radians = glm::radians(angle);
    sinTheta_ = glm::sin(radians);
    cosTheta_ = glm::cos(radians);
    aabb = hittable->aabb;

    glm::vec3 min(infinity, infinity, infinity);
    glm::vec3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i * aabb.x.max + (1 - i) * aabb.x.min;
                auto y = j * aabb.y.max + (1 - j) * aabb.y.min;
                auto z = k * aabb.z.max + (1 - k) * aabb.z.min;

                auto newx = cosTheta_ * x + sinTheta_ * z;
                auto newz = -sinTheta_ * x + cosTheta_ * z;

                glm::vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = glm::min(min[c], tester[c]);
                    max[c] = glm::max(max[c], tester[c]);
                }
            }
        }
    }

    aabb = AABB(min, max);
}

bool Rotate::hit(const Ray& ray, Interval t, HitRecord& hitRecord) const {
    auto origin = ray.origin;
    auto direction = ray.direction;

    origin[0] = cosTheta_ * ray.origin[0] - sinTheta_ * ray.origin[2];
    origin[2] = sinTheta_ * ray.origin[0] + cosTheta_ * ray.origin[2];
    direction[0] = cosTheta_ * ray.direction[0] - sinTheta_ * ray.direction[2];
    direction[2] = sinTheta_ * ray.direction[0] + cosTheta_ * ray.direction[2];

    Ray rayRotate(origin, direction, ray.time);

    if (!hittable_->hit(rayRotate, t, hitRecord)) {
        return false;
    }

    auto point = hitRecord.point;
    point[0] = cosTheta_ * hitRecord.point[0] + sinTheta_ * hitRecord.point[2];
    point[2] = -sinTheta_ * hitRecord.point[0] + cosTheta_ * hitRecord.point[2];

    auto normal = hitRecord.normal;
    normal[0] = cosTheta_ * hitRecord.normal[0] + sinTheta_ * hitRecord.normal[2];
    normal[2] = -sinTheta_ * hitRecord.normal[0] + cosTheta_ * hitRecord.normal[2];

    hitRecord.point = point;
    hitRecord.normal = normal;

    return true;
}