#include "RandomStreamGen.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

RandomStreamGen::RandomStreamGen(uint64_t seed, size_t stream_size, Mode mode)
    : seed_(seed), n_(stream_size), mode_(mode), rng_(seed) {
    stream_.reserve(n_);
    if (mode_ == Mode::Uniform) build_stream_uniform();
    else build_stream_mixed();
}

const std::vector<std::string>& RandomStreamGen::stream() const {
    return stream_;
}

std::vector<size_t> RandomStreamGen::split_indices_by_fraction(double step_fraction) const {
    std::vector<size_t> idx;
    if (step_fraction <= 0.0) return idx;
    if (step_fraction > 1.0) step_fraction = 1.0;
    double cur = step_fraction;
    while (cur < 1.0 + 1e-12) {
        size_t k = static_cast<size_t>(std::floor(cur * static_cast<double>(n_)));
        if (k > n_) k = n_;
        if (idx.empty() || idx.back() != k) idx.push_back(k);
        cur += step_fraction;
        if (cur > 1.0 && cur < 1.0 + step_fraction) cur = 1.0;
        if (idx.back() == n_) break;
    }
    if (idx.empty() || idx.back() != n_) idx.push_back(n_);
    return idx;
}

std::vector<std::string_view> RandomStreamGen::prefix_view_by_fraction(double fraction) const {
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;
    size_t k = static_cast<size_t>(std::floor(fraction * static_cast<double>(n_)));
    if (k > n_) k = n_;
    std::vector<std::string_view> out;
    out.reserve(k);
    for (size_t i = 0; i < k; ++i) out.emplace_back(stream_[i]);
    return out;
}

bool RandomStreamGen::is_allowed_char(char c) {
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '-') return true;
    return false;
}

std::string RandomStreamGen::sanitize(std::string s) {
    for (char& c : s) {
        if (!is_allowed_char(c)) c = '-';
    }
    if (s.size() > 30) s.resize(30);
    return s;
}

char RandomStreamGen::rand_char() {
    static const std::string alphabet =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "-";
    std::uniform_int_distribution<size_t> dist(0, alphabet.size() - 1);
    return alphabet[dist(rng_)];
}

std::string RandomStreamGen::rand_string(size_t max_len) {
    std::uniform_int_distribution<size_t> len_dist(1, std::min<size_t>(30, max_len));
    size_t len = len_dist(rng_);
    std::string s;
    s.resize(len);
    for (size_t i = 0; i < len; ++i) s[i] = rand_char();
    return s;
}

std::string RandomStreamGen::dict_like_key() {
    std::uniform_int_distribution<int> kind(0, 2);
    int k = kind(rng_);
    std::uniform_int_distribution<uint32_t> iddist(0, 999999);
    uint32_t id = iddist(rng_);
    std::ostringstream oss;
    if (k == 0) oss << "user";
    else if (k == 1) oss << "item";
    else oss << "sess";
    oss << std::setw(6) << std::setfill('0') << id;
    return sanitize(oss.str());
}

std::string RandomStreamGen::hot_key(size_t bucket) {
    std::ostringstream oss;
    oss << "hot-" << bucket;
    return sanitize(oss.str());
}

void RandomStreamGen::build_stream_uniform() {
    for (size_t i = 0; i < n_; ++i) {
        stream_.push_back(rand_string(30));
    }
}

void RandomStreamGen::build_stream_mixed() {
    std::uniform_real_distribution<double> u(0.0, 1.0);
    std::uniform_int_distribution<size_t> hot_bucket(0, 127);
    for (size_t i = 0; i < n_; ++i) {
        double p = u(rng_);
        if (p < 0.70) {
            stream_.push_back(rand_string(30));
        } else if (p < 0.90) {
            stream_.push_back(dict_like_key());
        } else {
            stream_.push_back(hot_key(hot_bucket(rng_)));
        }
    }
}
