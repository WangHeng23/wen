#pragma once

#include "resources/ray.hpp"
#include "tools/interval.hpp"
#include "hittable/hit_record.hpp"
#include "resources/aabb.hpp"

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const = 0;
    AABB aabb = AABB::empty;
};

class HittableList : public Hittable {
public:
    HittableList() = default;
    HittableList(const std::shared_ptr<Hittable>& hittable) { add(hittable); }

    bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const override {
        bool hitted = false;

        float tmax = t.max;
        HitRecord temp;
        for (const auto& hittable : hittables) {
            if (hittable->hit(ray, Interval(t.min, tmax), temp)) {
                hitted = true;
                tmax = temp.t;
                hitRecord = temp;
            }
        }

        return hitted;
    }

    void add(const std::shared_ptr<Hittable>& hittable) {
        hittables.push_back(hittable);
        aabb = AABB(aabb, hittable->aabb);
    }

    void clear() {
        hittables.clear();
    }

    std::vector<std::shared_ptr<Hittable>> hittables;
};