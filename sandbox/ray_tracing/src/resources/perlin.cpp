#include "resources/perlin.hpp"
#include "tools/random.hpp"

Perlin::Perlin() {
    ranvec_ = new glm::vec3[pointCount_];
    for (int i = 0; i < pointCount_; i++) {
        ranvec_[i] = Random::UnitSphere();
    }

    permX_ = perlinGenerate();
    permY_ = perlinGenerate();
    permZ_ = perlinGenerate();
}

Perlin::~Perlin() {
    delete[] ranvec_;
    delete[] permX_;
    delete[] permY_;
    delete[] permZ_;
}

float Perlin::noise(const glm::vec3& p) const {
    auto u = p.x - std::floor(p.x);
    auto v = p.y - std::floor(p.y);
    auto w = p.z - std::floor(p.z);
    auto i = static_cast<int>(std::floor(p.x));
    auto j = static_cast<int>(std::floor(p.y));
    auto k = static_cast<int>(std::floor(p.z));
    glm::vec3 c[2][2][2];

    for (int di = 0; di < 2; di++) {
        for (int dj = 0; dj < 2; dj++) {
            for (int dk = 0; dk < 2; dk++) {
                c[di][dj][dk] = ranvec_[
                    permX_[(i + di) & 255] ^
                    permY_[(j + dj) & 255] ^
                    permZ_[(k + dk) & 255]
                ];
            }
        }
    }

    return perlinInterp(c, u, v, w);
}

float Perlin::turb(const glm::vec3& p, int depth) const {
    auto accum = 0.0f;
    auto tempP = p;
    auto weight = 1.0f;

    for (int i = 0; i < depth; i++) {
        accum += weight * noise(tempP);
        weight *= 0.5f;
        tempP *= 2.0f;
    }

    return std::fabs(accum);
}

int* Perlin::perlinGenerate() {
    auto p = new int[pointCount_];
    for (int i = 0; i < pointCount_; i++) {
        p[i] = i;
    }

    permute(p, pointCount_);
    return p;
}

void Perlin::permute(int* p, int n) {
    for (int i = n - 1; i > 0; i--) {
        int target = Random::UInt(0, i);
        int tmp = p[i];
        p[i] = p[target];
        p[target] = tmp;
    }
}

float Perlin::perlinInterp(const glm::vec3 c[2][2][2], float u, float v, float w) {
    auto uu = u * u * (3.0f - 2.0f * u);
    auto vv = v * v * (3.0f - 2.0f * v);
    auto ww = w * w * (3.0f - 2.0f * w);
    auto accum = 0.0f;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                glm::vec3 weight(u - i, v - j, w - k);
                accum += (i * uu + (1 - i) * (1 - uu)) *
                         (j * vv + (1 - j) * (1 - vv)) *
                         (k * ww + (1 - k) * (1 - ww)) *
                         glm::dot(c[i][j][k], weight);
            }
        }
    }

    return accum;
}