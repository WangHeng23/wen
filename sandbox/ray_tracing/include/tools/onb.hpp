#pragma once

#include <glm/glm.hpp>

class ONB {
public:
    ONB() = default;

    glm::vec3 operator[](int i) const { return axis_[i]; }
    glm::vec3& operator[](int i) { return axis_[i]; }

    glm::vec3 u() const { return axis_[0]; }
    glm::vec3 v() const { return axis_[1]; }
    glm::vec3 w() const { return axis_[2]; }
    
    glm::vec3 local(float a, float b, float c) const {
        return a * axis_[0] + b * axis_[1] + c * axis_[2];
    }

    glm::vec3 local(const glm::vec3& a) const {
        return a.x * axis_[0] + a.y * axis_[1] + a.z * axis_[2];
    }

    void build(const glm::vec3& W) {
        glm::vec3 w = glm::normalize(W);
        glm::vec3 a = (glm::abs(w.x) > 0.9f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 v = glm::normalize(glm::cross(w, a));
        glm::vec3 u = glm::cross(w, v);
        axis_[0] = u;
        axis_[1] = v;
        axis_[2] = w;
    }

private:
    glm::vec3 axis_[3];
};