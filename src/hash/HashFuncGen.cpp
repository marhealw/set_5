#include "HashFuncGen.hpp"

HashFuncGen::HashFuncGen(uint64_t seed) : seed_(seed) {}

uint64_t HashFuncGen::fnv1a64(std::string_view s) {
    uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= static_cast<uint64_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}

uint64_t HashFuncGen::splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ull;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
    x = x ^ (x >> 31);
    return x;
}

uint32_t HashFuncGen::operator()(std::string_view s) const {
    uint64_t base = fnv1a64(s);
    uint64_t mixed = splitmix64(base ^ seed_);
    return static_cast<uint32_t>(mixed & 0xffffffffu);
}
