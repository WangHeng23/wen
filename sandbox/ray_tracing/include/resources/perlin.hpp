#pragma once

#include <glm/glm.hpp>

class Perlin {
public:
    Perlin();
    ~Perlin();

    float noise(const glm::vec3& p) const;
    float turb(const glm::vec3& p, int depth) const;

private:
    static int* perlinGenerate();
    static void permute(int* p, int n);
    static float perlinInterp(const glm::vec3 c[2][2][2], float u, float v, float w);

private:
    static const int pointCount_ = 256;
    glm::vec3* ranvec_;
    int* permX_;
    int* permY_;
    int* permZ_;
};