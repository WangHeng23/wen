#include "resources/pdf.hpp"
#include "tools/random.hpp"
#include <glm/ext/scalar_constants.hpp>

// CosinePDF
CosinePDF::CosinePDF(const glm::vec3& w) {
    uvw_.build(w);
}

float CosinePDF::value(const glm::vec3& direction) const {
    float cosTheta = glm::dot(glm::normalize(direction), uvw_.w());
    return glm::max(0.0f, cosTheta / glm::pi<float>());
}

glm::vec3 CosinePDF::generate() const {
    return uvw_.local(CosineDirection());
}

glm::vec3 CosinePDF::CosineDirection() {
    auto r1 = Random::Float();
    auto r2 = Random::Float();
    auto phi = 2.0f * glm::pi<float>() * r1;
    auto x = glm::cos(phi) * glm::sqrt(r2);
    auto y = glm::sin(phi) * glm::sqrt(r2);
    auto z = glm::sqrt(1.0f - r2);
    return glm::vec3(x, y, z);
}

// SpherePDF
float SpherePDF::value(const glm::vec3& direction) const {
    return 1.0f / (4.0f * glm::pi<float>());
}

glm::vec3 SpherePDF::generate() const {
    return Random::UnitSphere();
}

// HittablePDF
HittablePDF::HittablePDF(const std::shared_ptr<Hittable>& hittable, const glm::vec3& origin)
    : hittable_(hittable), origin_(origin) {}

float HittablePDF::value(const glm::vec3& direction) const {
    return hittable_->pdfValue(origin_, direction);
}

glm::vec3 HittablePDF::generate() const {
    return hittable_->random(origin_);
}

// MixturePDF
MixturePDF::MixturePDF(std::shared_ptr<PDF> p0, std::shared_ptr<PDF> p1) {
    p_[0] = p0;
    p_[1] = p1;
}

float MixturePDF::value(const glm::vec3& direction) const {
    return 0.5f * p_[0]->value(direction) + 0.5f * p_[1]->value(direction);
}

glm::vec3 MixturePDF::generate() const {
    if (Random::Float() < 0.5f) {
        return p_[0]->generate();
    } else {
        return p_[1]->generate();
    }
}