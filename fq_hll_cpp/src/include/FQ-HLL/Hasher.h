/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ Header file               *
 * File: Hasher.h                           *
 ****************************************** */

// ======== INCLUDE ======== //
#pragma once
#include <string>
#include <cstdint>
#include <cstring>

#if __has_include(<bit>) && defined(__cplusplus) && __cplusplus >= 202002L
    #include <bit> // For std::endian, std::byteswap (C++20)
#endif

// ======== FUNCTION PROTOTYPES ======== //
uint64_t str_to_u64(const std::string& str);