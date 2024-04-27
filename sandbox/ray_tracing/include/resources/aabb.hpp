#pragma once

#include "tools/interval.hpp"
#include "resources/ray.hpp"

class AABB {
public:
    AABB() = default;
    AABB(const Interval& x, const Interval& y, const Interval& z);
    AABB(const glm::vec3& a, const glm::vec3& b);
    AABB(const AABB& a, const AABB& b);

    bool hit(const Ray& ray, Interval t) const;

    const Interval& axis(int i) const;
    int longestAxis() const;

    Interval x, y, z;
    static const AABB empty, universe;

private:
    void expand();
};