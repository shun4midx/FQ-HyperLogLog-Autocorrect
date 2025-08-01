/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ file                      *
 * File: Hasher.cpp                         *
 ****************************************** */

// ======== INCLUDE ======== //
#include "../include/FQ-HLL/Hasher.h"

// ======= HASHER ======== //
uint64_t murmur3_64(const std::string& key, uint64_t seed = 42) {
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    size_t len = key.size();
    size_t nblocks = len / 8;

    uint64_t h = seed ^ (len * m);

    for (size_t i = 0; i < nblocks; ++i) {
        uint64_t k;
        std::memcpy(&k, key.data() + i*8, sizeof(k));

        // Big Endian support
        #if __has_include(<bit>) && defined(__cplusplus) && __cplusplus >= 202002L
            if constexpr (std::endian::native == std::endian::big) {
                k = std::byteswap(k);
            }
        #elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            k = __builtin_bswap64(k);
        #endif

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char* tail = reinterpret_cast<const unsigned char*>(key.data()) + nblocks * 8;
    uint64_t rem = 0;
    switch (len & 7) {
        case 7: rem |= uint64_t(tail[6]) << 48; [[fallthrough]];
        case 6: rem |= uint64_t(tail[5]) << 40; [[fallthrough]];
        case 5: rem |= uint64_t(tail[4]) << 32; [[fallthrough]];
        case 4: rem |= uint64_t(tail[3]) << 24; [[fallthrough]];
        case 3: rem |= uint64_t(tail[2]) << 16; [[fallthrough]];
        case 2: rem |= uint64_t(tail[1]) <<  8; [[fallthrough]];
        case 1: rem |= uint64_t(tail[0]);
                h ^= rem;
                h *= m;
    }

    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return h;
}

uint64_t str_to_u64(const std::string& str) {
    return murmur3_64(str);
}