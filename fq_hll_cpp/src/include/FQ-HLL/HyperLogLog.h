/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ Header file               *
 * File: HyperLogLog.h                      *
 ****************************************** */

// ======== INCLUDE ======== //
#pragma once
#include "Hasher.h"
#include <bit>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

// ======== STRUCTS AND CLASSES ======== //
struct SketchConfig {
    int b = 10;
    double alpha_override = -1.0;
    uint64_t mask = 0xFFFFFFFFFFFFFFFF;
};

class HyperLogLog {
public:
    explicit HyperLogLog();
    explicit HyperLogLog(const SketchConfig& _cfg);
    void insert(uint64_t hash);
    void insert(const std::string& str);
    void shifted_insert(uint64_t hash, int shift);
    void shifted_insert(const std::string& str, int shift);
    void merge(const HyperLogLog& other);
    double estimate() const;
    void reset();

private:
    // ~~~~~~~~ VARIABLES ~~~~~~~~ //
    SketchConfig cfg;
    int m;
    double alpha_m;
    std::vector<uint8_t> registers;

    // ~~~~~~~~ FUNCTIONS ~~~~~~~~ //
    double compute_alpha();
    uint8_t rho(uint64_t w_suffix) const;
    uint8_t count_zero_registers() const;
};