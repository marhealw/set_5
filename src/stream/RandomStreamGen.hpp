#pragma once
#include <string>
#include <vector>
#include <random>
#include <cstdint>

class RandomStreamGen {
public:
    enum class Mode {
        Uniform,
        Mixed
    };

    RandomStreamGen(uint64_t seed, size_t stream_size, Mode mode = Mode::Mixed);

    const std::vector<std::string>& stream() const;

    std::vector<size_t> split_indices_by_fraction(double step_fraction) const;
    std::vector<std::string_view> prefix_view_by_fraction(double fraction) const;

private:
    uint64_t seed_;
    size_t n_;
    Mode mode_;
    std::mt19937_64 rng_;
    std::vector<std::string> stream_;

    static bool is_allowed_char(char c);
    static std::string sanitize(std::string s);

    char rand_char();
    std::string rand_string(size_t max_len);

    std::string dict_like_key();
    std::string hot_key(size_t bucket);

    void build_stream_uniform();
    void build_stream_mixed();
};
