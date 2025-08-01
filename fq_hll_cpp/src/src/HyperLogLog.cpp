/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ file                      *
 * File: HyperLogLog.cpp                    *
 ****************************************** */

// ======== INCLUDE ======== //
#include "../include/FQ-HLL/HyperLogLog.h"

// ======== FALLBACK ======== //
uint8_t clz(uint64_t num) {
    if (num == 0) {
        return 64;
    }
#if __cplusplus >= 202002L && defined(__cpp_lib_bit_ops) && __cpp_lib_bit_ops >= 201907L
    // C++20 and <bit> header with bit operations are available
    return (uint8_t)(std::countl_zero(num));
#elif defined(__GNUC__) || defined(__clang__)
    // Fallback to GCC/Clang intrinsic
    return (uint8_t)(__builtin_clzll(num));
#else
    // Basic fallback (less efficient)
    uint8_t count = 0;
    uint64_t mask = 1ULL << 63;
    while (mask > 0 && (num & mask) == 0) {
        ++count;
        mask >>= 1;
    }
    return count;
#endif
}

// ======== HyperLogLog CLASS IMPLEMENTATION ======== //
HyperLogLog::HyperLogLog() : HyperLogLog(SketchConfig()) {}

HyperLogLog::HyperLogLog(const SketchConfig& _cfg) {
    if (_cfg.b > 16) { // Practical bounds
        throw std::invalid_argument("Precision b must be less than 16.");
    }

    cfg = _cfg;
    m = 1 << cfg.b;
    alpha_m = compute_alpha();
    registers.assign(m, 0);
}

// ======== PRIVATE ======= //
double HyperLogLog::compute_alpha() {
    // Override
    if (cfg.alpha_override > 0) {
        return cfg.alpha_override;
    }

    // Otherwise
    if (m == 16) {
        return 0.673;
    } else if (m == 32) {
        return 0.697;
    } else if (m == 64) {
        return 0.709;
    } else {
        return 0.7213 / (1 + 1.079 / m);
    }
}

uint8_t HyperLogLog::rho(uint64_t w_suffix) const {
    if (w_suffix == 0) {
        return 64;
    }
    return clz(w_suffix) + 1;
}

uint8_t HyperLogLog::count_zero_registers() const {
    return std::count_if(registers.begin(), registers.end(), [](uint8_t r) { return r == 0; });
}

// ======== PUBLIC ======== //
void HyperLogLog::insert(uint64_t hash) {
    uint64_t j = hash >> (64 - cfg.b);
    uint64_t w = ((hash & cfg.mask) << cfg.b) & 0xFFFFFFFFFFFFFFFFULL;

    int r = rho(w);
    if (r > registers[j]) {
        registers[j] = r;
    }
}

void HyperLogLog::insert(const std::string& str) {
    insert(str_to_u64(str));
}

void HyperLogLog::shifted_insert(uint64_t hash, int shift) {
    uint64_t j = hash >> (64 - cfg.b);
    uint64_t w = ((hash & cfg.mask) << cfg.b) & 0xFFFFFFFFFFFFFFFFULL;;

    int r = rho(w) + shift;
    if (r > registers[j]) {
        registers[j] = r;
    }
}

void HyperLogLog::shifted_insert(const std::string& str, int shift) {
    shifted_insert(str_to_u64(str), shift);
}

void HyperLogLog::merge(const HyperLogLog& other) {
    if (m != other.m) {
        throw std::invalid_argument("Cannot merge HLLs with different number of registers");
    }

    for (int i = 0; i < m; ++i) {
        registers[i] = std::max(registers[i], other.registers[i]);
    }
}

double HyperLogLog::estimate() const {
    double TWO64 = exp2(64.0);

    double Z = 0;
    for (uint8_t r : registers) {
        Z += std::pow(2.0, -r);
    }

    double E = alpha_m * m * m / Z;
    double V = count_zero_registers();

    if (E <= 2.5 * m) {
        return (V != 0 ? m * log(m / V) : E);
    } else if (E <= 1.0 / 30 * TWO64) {
        return E;
    } else {
        return -1 * TWO64 * log(1.0 - E / TWO64);
    }
}

void HyperLogLog::reset() {
    std::fill(registers.begin(), registers.end(), 0);
}