#pragma once

#include "hittable/hittable.hpp"

class Translate : public Hittable {
public:
    Translate(const std::shared_ptr<Hittable>& hittable, const glm::vec3& displacement);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const override;

private:
    std::shared_ptr<Hittable> hittable_;
    glm::vec3 offset_;
};

class Rotate : public Hittable {
public:
    Rotate(const std::shared_ptr<Hittable>& hittable, float angle);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const override;

private:
    std::shared_ptr<Hittable> hittable_;
    float sinTheta_;
    float cosTheta_;
};