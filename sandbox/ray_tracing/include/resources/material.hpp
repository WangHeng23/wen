#pragma once

#include "resources/ray.hpp"
#include "hittable/hit_record.hpp"
#include "tools/random.hpp"

class Material {
public:
    virtual ~Material() = default;
    virtual bool scatter(const Ray& rayIn, const HitRecord& hitRecord, glm::vec3& attenuation, Ray& rayOut) const = 0;
};

class Lambertian : public Material {
public:
    explicit Lambertian(const glm::vec3 albedo) : albedo(albedo) {}

    bool scatter(const Ray& rayIn, const HitRecord& hitRecord, glm::vec3& attenuation, Ray& rayOut) const override {
        attenuation = albedo;
        glm::vec3 direction = hitRecord.normal + glm::normalize(Random::Vec3(-1.0f, 1.0f));
        rayOut = Ray(hitRecord.point, glm::normalize(direction));
        return true; 
    }

    glm::vec3 albedo;
};

class Metal : public Material {
public:
    Metal(const glm::vec3& albedo, float roughness) : albedo(albedo), roughness(roughness) {}

    bool scatter(const Ray& rayIn, const HitRecord& hitRecord, glm::vec3& attenuation, Ray& rayOut) const override {
        attenuation = albedo;
        glm::vec3 reflected = glm::reflect(glm::normalize(rayIn.direction), hitRecord.normal);
        auto direction = reflected + roughness * glm::normalize(Random::Vec3(-1.0f, 1.0f));
        rayOut = Ray(hitRecord.point, glm::normalize(direction));
        return glm::dot(rayOut.direction, hitRecord.normal) > 0;
    }

    glm::vec3 albedo;
    float roughness;
};

class Dielectric : public Material {
public:
    explicit Dielectric(float ir) : ir(ir) {}

    bool scatter(const Ray& rayIn, const HitRecord& hitRecord, glm::vec3& attenuation, Ray& rayOut) const override {
        attenuation = glm::vec3(1.0f);
        float refractionRatio = hitRecord.inside ? (1.0f / ir) : ir;
        glm::vec3 unitDirection = glm::normalize(rayIn.direction);
        float cosTheta = glm::min(glm::dot(-unitDirection, hitRecord.normal), 1.0f);
        float sinTheta = glm::sqrt(1.0f - cosTheta * cosTheta);

        glm::vec3 direction;
        if (refractionRatio * sinTheta > 1.0f || reflectance(cosTheta, ir) > Random::Float()) {
            direction = glm::reflect(unitDirection, hitRecord.normal);
        } else {
            direction = glm::refract(unitDirection, hitRecord.normal, refractionRatio);
        }

        rayOut = Ray(hitRecord.point, glm::normalize(direction));
        return true;
    }

    float ir;
    static double reflectance(float cosTheta, float ir) {
        auto r0 = (1 - ir) / (1 + ir);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosTheta), 5);
    }
};