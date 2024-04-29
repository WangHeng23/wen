#pragma once

#include "hittable/hittable.hpp"
#include "tools/onb.hpp"

class PDF {
public:
    virtual ~PDF() = default;
    virtual float value(const glm::vec3& direction) const = 0;
    virtual glm::vec3 generate() const = 0;
};

class CosinePDF : public PDF {
public:
    CosinePDF(const glm::vec3& w);

    float value(const glm::vec3& direction) const override;
    glm::vec3 generate() const override;

private:
    static glm::vec3 CosineDirection();

private:
    ONB uvw_;
};

class SpherePDF : public PDF {
public:
    SpherePDF() = default;
    
    float value(const glm::vec3& direction) const override;
    glm::vec3 generate() const override;
};

class HittablePDF : public PDF {
public:
    HittablePDF(const std::shared_ptr<Hittable>& hittable, const glm::vec3& origin);
    
    float value(const glm::vec3& direction) const override;
    glm::vec3 generate() const override;

private:
    std::shared_ptr<Hittable> hittable_;
    glm::vec3 origin_;
};

class MixturePDF : public PDF {
public:
    MixturePDF(std::shared_ptr<PDF> p0, std::shared_ptr<PDF> p1);

    float value(const glm::vec3& direction) const override;
    glm::vec3 generate() const override;

private:
    std::shared_ptr<PDF> p_[2];
};