#include "HyperLogLog.hpp"
#include <cmath>
#include <algorithm>

HyperLogLog::HyperLogLog(uint8_t B) : B_(B), m_(1u << B), regs_(m_, 0) {}

uint8_t HyperLogLog::B() const { return B_; }
uint32_t HyperLogLog::m() const { return m_; }
const std::vector<uint8_t>& HyperLogLog::regs() const { return regs_; }

double HyperLogLog::alpha(uint32_t m) {
    if (m == 16) return 0.673;
    if (m == 32) return 0.697;
    if (m == 64) return 0.709;
    return 0.7213 / (1.0 + 1.079 / static_cast<double>(m));
}

uint8_t HyperLogLog::rho(uint32_t w, uint8_t max_bits) {
    if (w == 0) return static_cast<uint8_t>(max_bits + 1);
#if defined(__GNUG__) || defined(__clang__)
    uint32_t lz = __builtin_clz(w);
    uint32_t bits = 32;
    uint32_t lead = lz;
    uint32_t r = lead + 1;
    if (r > max_bits + 1) r = max_bits + 1;
    return static_cast<uint8_t>(r);
#else
    uint8_t r = 1;
    uint32_t mask = 1u << 31;
    while (r <= max_bits && (w & mask) == 0) {
        r++;
        mask >>= 1;
    }
    if (r > max_bits + 1) r = max_bits + 1;
    return r;
#endif
}

void HyperLogLog::add(uint32_t x) {
    uint32_t idx = x >> (32 - B_);
    uint32_t w = x << B_;
    uint8_t max_bits = static_cast<uint8_t>(32 - B_);
    uint8_t r = rho(w, max_bits);
    if (r > regs_[idx]) regs_[idx] = r;
}

double HyperLogLog::estimate() const {
    double z = 0.0;
    for (uint8_t v : regs_) {
        z += std::ldexp(1.0, -static_cast<int>(v));
    }
    double E = alpha(m_) * static_cast<double>(m_) * static_cast<double>(m_) / z;
    return E;
}
