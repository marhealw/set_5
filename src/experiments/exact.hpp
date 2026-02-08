#pragma once
#include <unordered_set>
#include <string_view>
#include <vector>
#include <cstdint>

struct SvHash {
    size_t operator()(std::string_view s) const noexcept {
        uint64_t h = 1469598103934665603ull;
        for (unsigned char c : s) {
            h ^= static_cast<uint64_t>(c);
            h *= 1099511628211ull;
        }
        return static_cast<size_t>(h);
    }
};

struct SvEq {
    bool operator()(std::string_view a, std::string_view b) const noexcept {
        return a == b;
    }
};

inline uint64_t exact_F0(const std::vector<std::string_view>& prefix) {
    std::unordered_set<std::string_view, SvHash, SvEq> st;
    st.reserve(prefix.size());
    for (auto s : prefix) st.insert(s);
    return static_cast<uint64_t>(st.size());
}
