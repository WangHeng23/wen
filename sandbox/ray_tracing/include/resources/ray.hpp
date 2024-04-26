#pragma once

#include <glm/glm.hpp>

class Ray {
public:
    Ray() = default;
    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(direction) {}
    
    [[nodiscard]] glm::vec3 hitPoint(float t) const {
        return origin + t * direction;
    }

    glm::vec3 origin;
    glm::vec3 direction;
};