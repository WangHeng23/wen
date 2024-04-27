#include "hittable/bvh.hpp"

BVH::BVH(const std::vector<std::shared_ptr<Hittable>>& src, size_t start, size_t end) {
    aabb = AABB::empty;
    for (size_t i = start; i < end; i++) {
        aabb = AABB(aabb, src[i]->aabb);
    }

    int axis = aabb.longestAxis();
    auto comparator = (axis == 0) ? x : (axis == 1) ? y : z;

    auto objects = src;
    size_t size = end - start;
    if (size == 1) {
        left = right = objects[start];
    } else if (size == 2) {
        if (comparator(objects[start], objects[start + 1])) {
            left = objects[start];
            right = objects[start + 1];
        } else {
            left = objects[start + 1];
            right = objects[start];
        }
    } else {
        std::sort(objects.begin() + start, objects.begin() + end, comparator);
        size_t mid = start + size / 2;
        left = std::make_shared<BVH>(objects, start, mid);
        right = std::make_shared<BVH>(objects, mid, end);
    }
}

BVH::BVH(const std::shared_ptr<HittableList>& list) : BVH(list->hittables, 0, list->hittables.size()) {}

bool BVH::hit(const Ray& ray, Interval t, HitRecord& hitRecoed) const {
    if (!aabb.hit(ray, t)) {
        return false;
    }

    bool _left = left->hit(ray, t, hitRecoed);
    bool _right = right->hit(ray, Interval(t.min, _left ? hitRecoed.t : t.max), hitRecoed);

    return _left || _right;
}

bool BVH::compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b, int index) {
    return a->aabb.axis(index).min < b->aabb.axis(index).min;
}

bool BVH::x(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
    return compare(a, b, 0);
}

bool BVH::y(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
    return compare(a, b, 1);
}

bool BVH::z(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
    return compare(a, b, 2);
}