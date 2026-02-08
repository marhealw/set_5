#pragma once
#include <cstdint>
#include <string_view>

class HashFuncGen {
public:
    explicit HashFuncGen(uint64_t seed);

    uint32_t operator()(std::string_view s) const;

private:
    uint64_t seed_;

    static uint64_t fnv1a64(std::string_view s);
    static uint64_t splitmix64(uint64_t x);
};
