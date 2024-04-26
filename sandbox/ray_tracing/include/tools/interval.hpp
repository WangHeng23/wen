#pragma once

const float infinity = std::numeric_limits<float>::infinity();

class Interval {
public:
    Interval() : min(+infinity), max(-infinity) {}
    Interval(float min, float max) : min(min), max(max) {}
    Interval(const Interval& a, const Interval& b) : min(fmin(a.min, b.min)), max(fmax(a.max, b.max)) {}

    float size() const { return max - min; }
    bool contains(float x) const { return min <= x && x <= max; }
    bool inside(float x) const { return min < x && x < max; }

    float min, max;
    static const Interval empty, universe;
};