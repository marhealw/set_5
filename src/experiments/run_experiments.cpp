#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <iomanip>
#include <cmath>
#include <numeric>

#include "stream/RandomStreamGen.hpp"
#include "hash/HashFuncGen.hpp"
#include "hll/HyperLogLog.hpp"
#include "experiments/exact.hpp"
#include "experiments/choose_B.hpp"

namespace fs = std::filesystem;

struct Args {
    uint64_t seed = 123;
    size_t streams = 20;
    size_t n = 200000;
    double step = 0.05;
    int B = 14;
    std::string mode = "mixed";
    std::string out_dir = "data/runs";
};

static bool starts_with(const std::string& s, const std::string& p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

static Args parse(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string x = argv[i];
        auto getv = [&](const std::string& key) -> std::string {
            if (starts_with(x, key + "=")) return x.substr(key.size() + 1);
            if (x == key && i + 1 < argc) return std::string(argv[++i]);
            return "";
        };
        std::string v;
        if (!(v = getv("--seed")).empty()) a.seed = std::stoull(v);
        else if (!(v = getv("--streams")).empty()) a.streams = static_cast<size_t>(std::stoull(v));
        else if (!(v = getv("--n")).empty()) a.n = static_cast<size_t>(std::stoull(v));
        else if (!(v = getv("--step")).empty()) a.step = std::stod(v);
        else if (!(v = getv("--B")).empty()) a.B = std::stoi(v);
        else if (!(v = getv("--mode")).empty()) a.mode = v;
        else if (!(v = getv("--out")).empty()) a.out_dir = v;
    }
    if (a.step <= 0.0) a.step = 0.05;
    if (a.step > 1.0) a.step = 1.0;
    if (a.B < 4) a.B = 4;
    if (a.B > 18) a.B = 18;
    return a;
}

static RandomStreamGen::Mode to_mode(const std::string& s) {
    if (s == "uniform") return RandomStreamGen::Mode::Uniform;
    return RandomStreamGen::Mode::Mixed;
}

static std::vector<double> fractions(double step) {
    std::vector<double> f;
    double cur = step;
    while (cur < 1.0 + 1e-12) {
        if (cur > 1.0) cur = 1.0;
        f.push_back(cur);
        if (cur >= 1.0) break;
        cur += step;
    }
    if (f.empty() || std::abs(f.back() - 1.0) > 1e-12) f.push_back(1.0);
    return f;
}

static void ensure_dir(const std::string& p) {
    fs::path dir(p);
    if (!fs::exists(dir)) fs::create_directories(dir);
}

static void write_run_csv(const fs::path& path,
                          const std::vector<double>& frac,
                          const std::vector<uint64_t>& f0,
                          const std::vector<double>& nt) {
    std::ofstream out(path);
    out << "t,processed_fraction,true_F0,estimate_Nt,rel_error\n";
    for (size_t i = 0; i < frac.size(); ++i) {
        double re = 0.0;
        if (f0[i] > 0) re = (nt[i] - static_cast<double>(f0[i])) / static_cast<double>(f0[i]);
        out << (i + 1) << "," << std::fixed << std::setprecision(6) << frac[i] << ",";
        out << f0[i] << "," << std::fixed << std::setprecision(6) << nt[i] << ",";
        out << std::fixed << std::setprecision(6) << re << "\n";
    }
}

static void write_summary_csv(const fs::path& path,
                              const std::vector<double>& frac,
                              const std::vector<double>& true_mean,
                              const std::vector<double>& est_mean,
                              const std::vector<double>& est_sd) {
    std::ofstream out(path);
    out << "t,processed_fraction,mean_true_F0,mean_estimate_Nt,sigma_estimate_Nt\n";
    for (size_t i = 0; i < frac.size(); ++i) {
        out << (i + 1) << "," << std::fixed << std::setprecision(6) << frac[i] << ",";
        out << std::fixed << std::setprecision(6) << true_mean[i] << ",";
        out << std::fixed << std::setprecision(6) << est_mean[i] << ",";
        out << std::fixed << std::setprecision(6) << est_sd[i] << "\n";
    }
}

int main(int argc, char** argv) {
    Args args = parse(argc, argv);
    ensure_dir(args.out_dir);

    auto fr = fractions(args.step);
    size_t T = fr.size();

    std::vector<std::vector<uint64_t>> all_true(args.streams, std::vector<uint64_t>(T, 0));
    std::vector<std::vector<double>> all_est(args.streams, std::vector<double>(T, 0.0));

    for (size_t s = 0; s < args.streams; ++s) {
        uint64_t seed_stream = args.seed + 1000003ull * (s + 1);
        RandomStreamGen gen(seed_stream, args.n, to_mode(args.mode));
        HashFuncGen hf(seed_stream ^ 0x9e3779b97f4a7c15ull);
        HyperLogLog hll(static_cast<uint8_t>(args.B));

        const auto& stream = gen.stream();

        size_t prev_k = 0;
        for (size_t ti = 0; ti < T; ++ti) {
            double frac = fr[ti];
            size_t k = static_cast<size_t>(std::floor(frac * static_cast<double>(args.n)));
            if (k > args.n) k = args.n;

            for (size_t i = prev_k; i < k; ++i) {
                uint32_t x = hf(std::string_view(stream[i]));
                hll.add(x);
            }
            prev_k = k;

            std::vector<std::string_view> prefix;
            prefix.reserve(k);
            for (size_t i = 0; i < k; ++i) prefix.emplace_back(stream[i]);

            uint64_t f0 = exact_F0(prefix);
            double nt = hll.estimate();

            all_true[s][ti] = f0;
            all_est[s][ti] = nt;
        }

        fs::path run_path = fs::path(args.out_dir) / ("run_" + std::to_string(s + 1) + ".csv");
        std::vector<uint64_t> f0v(T);
        std::vector<double> ntv(T);
        for (size_t i = 0; i < T; ++i) {
            f0v[i] = all_true[s][i];
            ntv[i] = all_est[s][i];
        }
        write_run_csv(run_path, fr, f0v, ntv);

        std::cout << "saved " << run_path.string() << "\n";
    }

    std::vector<double> mean_true(T, 0.0), mean_est(T, 0.0), sd_est(T, 0.0);

    for (size_t ti = 0; ti < T; ++ti) {
        double sum_t = 0.0, sum_e = 0.0;
        for (size_t s = 0; s < args.streams; ++s) {
            sum_t += static_cast<double>(all_true[s][ti]);
            sum_e += all_est[s][ti];
        }
        mean_true[ti] = sum_t / static_cast<double>(args.streams);
        mean_est[ti] = sum_e / static_cast<double>(args.streams);

        double var = 0.0;
        for (size_t s = 0; s < args.streams; ++s) {
            double d = all_est[s][ti] - mean_est[ti];
            var += d * d;
        }
        var /= static_cast<double>(args.streams);
        sd_est[ti] = std::sqrt(var);
    }

    fs::path summary_path = fs::path("data") / "summary.csv";
    ensure_dir("data");
    write_summary_csv(summary_path, fr, mean_true, mean_est, sd_est);
    std::cout << "saved " << summary_path.string() << "\n";

    uint32_t m = 1u << static_cast<uint8_t>(args.B);
    double theo1 = 1.042 / std::sqrt(static_cast<double>(m));
    double theo2 = 1.32 / std::sqrt(static_cast<double>(m));
    std::cout << "B=" << args.B << " m=" << m << " theo_rse_1=" << theo1 << " theo_rse_2=" << theo2 << "\n";
    return 0;
}
