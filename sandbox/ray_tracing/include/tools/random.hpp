#pragma once

#include <random>
#include <glm/glm.hpp>

class Random {
public:
    static void Init() {
        randomEngine.seed(std::random_device()());
    }

    static uint32_t UInt() {
        return distribution(randomEngine);
    }

    // [min, max]
    static uint32_t UInt(uint32_t min, uint32_t max) {
        return min + (distribution(randomEngine) % (max - min + 1));
    }

    // (0, 1)
    static float Float() {
        return (float)distribution(randomEngine) / (float)std::numeric_limits<uint32_t>::max();
    }

    // [min, max)
    static float Float(float min, float max) {
        return min + Float() * (max - min);
    }

    static glm::vec3 Vec3() {
        return glm::vec3(Float(), Float(), Float());
    }

    static glm::vec3 Vec3(float min, float max) {
        return glm::vec3(Float() * (max - min) + min, Float() * (max - min) + min, Float() * (max - min) + min);
    }

    static thread_local std::mt19937 randomEngine;
    static std::uniform_int_distribution<std::mt19937::result_type> distribution;
};