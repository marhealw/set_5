#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include <numeric>
#include <algorithm>

inline double bucket_cv(const std::vector<uint32_t>& counts) {
    double m = static_cast<double>(counts.size());
    double mean = std::accumulate(counts.begin(), counts.end(), 0.0) / m;
    if (mean == 0.0) return 0.0;
    double var = 0.0;
    for (auto c : counts) {
        double d = static_cast<double>(c) - mean;
        var += d * d;
    }
    var /= m;
    double sd = std::sqrt(var);
    return sd / mean;
}
