#include "tools/random.hpp"
#include "tools/interval.hpp"

thread_local std::mt19937 Random::randomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::distribution;

const Interval Interval::empty = Interval(+infinity, -infinity);
const Interval Interval::universe = Interval(-infinity, +infinity);