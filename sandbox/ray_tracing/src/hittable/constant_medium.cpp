#include "hittable/constant_medium.hpp"

ConstantMedium::ConstantMedium(const std::shared_ptr<Hittable>& boundary, float density, const std::shared_ptr<Texture>& albedo) {
    boundary_ = boundary;
    negInvDensity_ = -1.0f / density;
    phase_ = std::make_shared<Isotropic>(albedo);
}

ConstantMedium::ConstantMedium(const std::shared_ptr<Hittable>& boundary, float density, const glm::vec3& albedo) {
    boundary_ = boundary;
    negInvDensity_ = -1.0f / density;
    phase_ = std::make_shared<Isotropic>(albedo);
}

bool ConstantMedium::hit(const Ray& ray, Interval t, HitRecord& hitRecoed) const {
    HitRecord rec1, rec2;
    if (!boundary_->hit(ray, Interval::universe, rec1)) {
        return false;
    }
    if (!boundary_->hit(ray, Interval(rec1.t + 0.0001f, infinity), rec2)) {
        return false;
    }

    if (rec1.t < t.min) rec1.t = t.min;
    if (rec2.t > t.max) rec2.t = t.max;

    if (rec1.t >= rec2.t) {
        return false;
    }

    if (rec1.t < 0) {
        rec1.t = 0;
    }

    auto rayLength = glm::length(ray.direction);
    auto distanceInsideBoundary = (rec2.t - rec1.t) * rayLength;
    auto hitDistance = negInvDensity_ * log(Random::Float());

    if (hitDistance > distanceInsideBoundary) {
        return false;
    }

    hitRecoed.t = rec1.t + hitDistance / rayLength;
    hitRecoed.point = ray.hitPoint(hitRecoed.t);
    hitRecoed.normal = glm::vec3(1.0f, 0.0f, 0.0f);
    hitRecoed.inside = true;
    hitRecoed.material = phase_;

    return true;
}