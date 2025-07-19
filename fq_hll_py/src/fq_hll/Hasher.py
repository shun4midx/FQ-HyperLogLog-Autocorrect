############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: Hasher.py                          #
############################################

import struct

def murmur3_64(key: str, seed: int = 42) -> int:
    key_bytes = key.encode('utf-8')
    length = len(key_bytes)
    m = 0xc6a4a7935bd1e995
    r = 47

    h = seed ^ (length * m)

    num_blocks = length // 8
    for i in range(num_blocks):
        k = struct.unpack_from('<Q', key_bytes, i * 8)[0]  # Little-endian 64-bit
        k *= m
        k ^= k >> r
        k *= m

        h ^= k
        h *= m

    remaining = key_bytes[num_blocks * 8:]
    remaining_val = 0
    for i in range(len(remaining)):
        remaining_val |= remaining[i] << (i * 8)
    if remaining:
        h ^= remaining_val
        h *= m

    h ^= h >> r
    h *= m
    h ^= h >> r

    return h & 0xFFFFFFFFFFFFFFFF  # Return as unsigned 64-bit

def str_to_u64(s: str) -> int:
    return murmur3_64(s)