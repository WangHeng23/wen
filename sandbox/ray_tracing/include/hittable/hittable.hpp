#pragma once

#include "resources/ray.hpp"
#include "tools/interval.hpp"
#include "hittable/hit_record.hpp"

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const = 0;
};

class HittableList : public Hittable {
public:
    HittableList() = default;

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
    }

    void clear() {
        hittables.clear();
    }

    std::vector<std::shared_ptr<Hittable>> hittables;
};