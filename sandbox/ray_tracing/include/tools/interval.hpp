#pragma once

#include <glm/glm.hpp>

const float infinity = std::numeric_limits<float>::infinity();

class Interval {
public:
    Interval() : min(+infinity), max(-infinity) {}
    Interval(float min, float max) : min(min), max(max) {}
    Interval(const Interval& a, const Interval& b) : min(glm::min(a.min, b.min)), max(glm::max(a.max, b.max)) {}

    float size() const { return max - min; }
    bool contains(float x) const { return min <= x && x <= max; }
    bool inside(float x) const { return min < x && x < max; }

    Interval extend(float x) const { 
        return Interval(min - x * 0.5f, max + x * 0.5f);
    }

    float min, max;
    static const Interval empty, universe;
};