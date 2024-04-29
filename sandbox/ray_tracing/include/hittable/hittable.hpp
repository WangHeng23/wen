#pragma once

#include "resources/ray.hpp"
#include "tools/interval.hpp"
#include "hittable/hit_record.hpp"
#include "resources/aabb.hpp"
#include "tools/random.hpp"

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& ray, Interval t, HitRecord& hitRecord) const = 0;
    AABB aabb = AABB::empty;
    virtual float pdfValue(const glm::vec3& origin, const glm::vec3& direction) const { return 0.0f;}
    virtual glm::vec3 random(const glm::vec3& origin) const { return glm::vec3(1.0f, 0.0f, 0.0f); }
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

    float pdfValue(const glm::vec3& origin, const glm::vec3& direction) const override {
        float weight = 1.0f / hittables.size();
        float sum = 0.0f;
        for (const auto& hittable : hittables) {
            sum += weight * hittable->pdfValue(origin, direction);
        }
        return sum;
    }

    glm::vec3 random(const glm::vec3& origin) const override {
        int index = Random::UInt(0, hittables.size() - 1);
        return hittables[index]->random(origin);
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