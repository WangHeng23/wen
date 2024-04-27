#include "resources/aabb.hpp"
#include "tools/interval.hpp"

const AABB AABB::empty = AABB(Interval::empty, Interval::empty, Interval::empty);
const AABB AABB::universe = AABB(Interval::universe, Interval::universe, Interval::universe);

AABB::AABB(const Interval& x, const Interval& y, const Interval& z) {
    this->x = x;
    this->y = y;
    this->z = z;
    expand();
}

AABB::AABB(const glm::vec3& a, const glm::vec3& b) {
    x = Interval(glm::min(a.x, b.x), glm::max(a.x, b.x));
    y = Interval(glm::min(a.y, b.y), glm::max(a.y, b.y));
    z = Interval(glm::min(a.z, b.z), glm::max(a.z, b.z));
    expand();
}

AABB::AABB(const AABB& a, const AABB& b) {
    x = Interval(a.x, b.x);
    y = Interval(a.y, b.y);
    z = Interval(a.z, b.z);
    expand();
}

bool AABB::hit(const Ray& ray, Interval t) const {
    for (int i = 0; i < 3; i++) {
        float origin = ray.origin[i];
        float direction = ray.direction[i];

        float t0 = (axis(i).min - origin) / direction;
        float t1 = (axis(i).max - origin) / direction;

        if (direction < 0) {
            std::swap(t0, t1);
        }

        if (t0 > t.min) t.min = t0;
        if (t1 < t.max) t.max = t1;

        if (t.max <= t.min) {
            return false;
        }
    }

    return true;
}

const Interval& AABB::axis(int i) const {
    return i == 0 ? x : (i == 1 ? y : z);
}

int AABB::longestAxis() const {
    return x.size() > y.size() ? (x.size() > z.size() ? 0 : 2) : (y.size() > z.size() ? 1 : 2);
}

void AABB::expand() {
    float delta = 0.0001f;
    if (x.size() < delta) x = x.extend(delta);
    if (y.size() < delta) y = y.extend(delta);
    if (z.size() < delta) z = z.extend(delta);
}