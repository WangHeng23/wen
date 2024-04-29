#pragma once

#include "resources/ray.hpp"
#include "hittable/hit_record.hpp"
#include "tools/random.hpp"
#include "resources/textures.hpp"
#include "resources/pdf.hpp"
#include <glm/ext/scalar_constants.hpp>

class ScatterRecord {
public:
    glm::vec3 attenuation;
    std::shared_ptr<PDF> pdf;
    Ray rayOut;
};

class Material {
public:
    virtual ~Material() = default;

    virtual bool scatter(const Ray& rayIn, const HitRecord& hitRecord, ScatterRecord& scatterRecord) const {
        return false;
    }

    virtual glm::vec3 emitted(const HitRecord& hitRecord) const {
        return glm::vec3(0.0f);
    }

    virtual float pdf(const HitRecord& hitRecord, const Ray& rayOut) const {
        return 0.0f;
    }
};

class Lambertian : public Material {
public:
    explicit Lambertian(const glm::vec3& albedo) : albedo(std::make_shared<SolidColor>(albedo)) {}
    explicit Lambertian(const std::shared_ptr<Texture>& albedo) : albedo(albedo) {}

    bool scatter(const Ray& rayIn, const HitRecord& hitRecord, ScatterRecord& scatterRecord) const override {
        scatterRecord.attenuation = albedo->value(hitRecord.u, hitRecord.v, hitRecord.point);
        scatterRecord.pdf = std::make_shared<CosinePDF>(hitRecord.normal);

        auto direction = hitRecord.normal + Random::UnitSphere();
        scatterRecord.rayOut = Ray(hitRecord.point, direction, rayIn.time);
        return true; 
    }

    float pdf(const HitRecord& hitRecord, const Ray& rayOut) const override {
        float cosTheta = glm::dot(hitRecord.normal, rayOut.direction);
        return glm::max(0.0f, cosTheta / glm::pi<float>());
    }

    std::shared_ptr<Texture> albedo;
};

class Metal : public Material {
public:
    Metal(const glm::vec3& albedo, float roughness) : albedo(albedo), roughness(roughness) {}

    bool scatter(const Ray& rayIn, const HitRecord& hitRecord, ScatterRecord& scatterRecord) const override {
        scatterRecord.attenuation = albedo;
        scatterRecord.pdf = nullptr;

        glm::vec3 reflected = glm::reflect(glm::normalize(rayIn.direction), hitRecord.normal);
        auto direction = reflected + roughness * glm::normalize(Random::Vec3(-1.0f, 1.0f));
        scatterRecord.rayOut = Ray(hitRecord.point, glm::normalize(direction), rayIn.time);
        return true;
    }

    glm::vec3 albedo;
    float roughness;
};

class Dielectric : public Material {
public:
    explicit Dielectric(float ir) : ir(ir) {}

    bool scatter(const Ray& rayIn, const HitRecord& hitRecord, ScatterRecord& scatterRecord) const override {
        scatterRecord.attenuation = glm::vec3(1.0f);
        scatterRecord.pdf = nullptr;

        float refractionRatio = hitRecord.inside ? (1.0f / ir) : ir;
        glm::vec3 unitDirection = glm::normalize(rayIn.direction);
        float cosTheta = glm::min(glm::dot(-unitDirection, hitRecord.normal), 1.0f);
        float sinTheta = glm::sqrt(1.0f - cosTheta * cosTheta);

        glm::vec3 direction;
        if (refractionRatio * sinTheta > 1.0f || reflectance(cosTheta, refractionRatio) > Random::Float()) {
            direction = glm::reflect(unitDirection, hitRecord.normal);
        } else {
            direction = glm::refract(unitDirection, hitRecord.normal, refractionRatio);
        }

        scatterRecord.rayOut = Ray(hitRecord.point, direction, rayIn.time);
        return true;
    }

    float ir;
    static double reflectance(float cosTheta, float ir) {
        auto r0 = (1 - ir) / (1 + ir);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosTheta), 5);
    }
};

class DiffuseLight : public Material {
public:
    DiffuseLight(const glm::vec3& emit) : emit(std::make_shared<SolidColor>(emit)) {}
    DiffuseLight(const std::shared_ptr<Texture>& emit) : emit(emit) {}

    glm::vec3 emitted(const HitRecord& hitRecord) const override {
        if (!hitRecord.inside) {
            return glm::vec3(0.0f);
        }
        return emit->value(hitRecord.u, hitRecord.v, hitRecord.point);
    }

    std::shared_ptr<Texture> emit;
};

class Isotropic : public Material {
public:
    Isotropic(const glm::vec3& albedo) : albedo(std::make_shared<SolidColor>(albedo)) {}
    Isotropic(const std::shared_ptr<Texture>& albedo) : albedo(albedo) {}

    bool scatter(const Ray& rayIn, const HitRecord& hitRecord, ScatterRecord& scatterRecord) const override {
        scatterRecord.attenuation = albedo->value(hitRecord.u, hitRecord.v, hitRecord.point);
        scatterRecord.pdf = std::make_shared<SpherePDF>();

        auto dircetion = Random::Vec3(-1.0f, 1.0f);
        scatterRecord.rayOut = Ray(hitRecord.point, glm::normalize(dircetion), rayIn.time); 
        return true;
    }

    float pdf(const HitRecord& hitRecord, const Ray& rayOut) const override {
        return 1.0f / (4.0f * glm::pi<float>());
    }

    std::shared_ptr<Texture> albedo;
};