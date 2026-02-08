#pragma once
#include <vector>
#include <cstdint>
#include <string_view>

class HyperLogLog {
public:
    explicit HyperLogLog(uint8_t B);

    void add(uint32_t x);
    double estimate() const;

    uint8_t B() const;
    uint32_t m() const;
    const std::vector<uint8_t>& regs() const;

private:
    uint8_t B_;
    uint32_t m_;
    std::vector<uint8_t> regs_;

    static double alpha(uint32_t m);
    static uint8_t rho(uint32_t w, uint8_t max_bits);
};
