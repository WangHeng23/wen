#pragma once

#include "hittable/hittable.hpp"

class BVH : public Hittable {
public:
    BVH(const std::vector<std::shared_ptr<Hittable>>& src, size_t start, size_t end);
    BVH(const std::shared_ptr<HittableList>& list);

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecoed) const override;

    std::shared_ptr<Hittable> left;
    std::shared_ptr<Hittable> right;

private:
    static bool compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b, int index);
    static bool x(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b);
    static bool y(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b);
    static bool z(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b);
};