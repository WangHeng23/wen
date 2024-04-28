#pragma once

#include "hittable/hittable.hpp"
#include "resources/material.hpp"
#include "resources/texture.hpp"

class ConstantMedium : public Hittable {
public:
    ConstantMedium(const std::shared_ptr<Hittable>& boundary, float density, const std::shared_ptr<Texture>& albedo);
    ConstantMedium(const std::shared_ptr<Hittable>& boundary, float density, const glm::vec3& albedo);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecoed) const override;

private:
    std::shared_ptr<Hittable> boundary_;
    std::shared_ptr<Material> phase_;
    float negInvDensity_;
};