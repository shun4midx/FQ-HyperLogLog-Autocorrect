############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: HyperLogLog.py                     #
############################################

import math
from .Hasher import str_to_u64

class SketchConfig:
    def __init__(self, b=10, alpha_override=-1.0):
        self.b = b
        self.alpha_override = alpha_override
        self.mask = (1 << (64 - self.b)) - 1

class HyperLogLog:
    def __init__(self, cfg=SketchConfig()):
        self.cfg = cfg
        self.m = 2 ** self.cfg.b
        self.alpha_m = self.compute_alpha()
        self.registers = bytearray(self.m)

    def compute_alpha(self):
        if self.cfg.alpha_override > 0:
            return self.cfg.alpha_override
        if self.m == 16:
            return 0.673
        elif self.m == 32:
            return 0.697
        elif self.m == 64:
            return 0.709
        else:
            return 0.7213 / (1 + 1.079 / self.m)

    def rho(self, w_suffix):
        if w_suffix == 0 or w_suffix >= (1 << 64):
            return 64
        return 64 - w_suffix.bit_length() + 1

    def insert(self, value):
        if isinstance(value, str):
            hash_val = str_to_u64(value)
        else:
            hash_val = value

        j = hash_val >> (64 - self.cfg.b)
        w = ((hash_val & self.cfg.mask) << self.cfg.b) & 0xFFFFFFFFFFFFFFFF
        
        r = self.rho(w)
        if r > self.registers[j]:
            self.registers[j] = r

    def shifted_insert(self, value, shift): # Not standard HLL, it's part of my FQ-HLL implementation
        if isinstance(value, str):
            hash_val = str_to_u64(value)
        else:
            hash_val = value

        j = hash_val >> (64 - self.cfg.b)
        w = ((hash_val & self.cfg.mask) << self.cfg.b) & 0xFFFFFFFFFFFFFFFF
        
        r = self.rho(w) + shift
        if r > self.registers[j]:
            self.registers[j] = r

    def merge(self, other):
        if self.m != other.m:
            raise ValueError("Cannot merge HLLs with different number of registers")
        for i in range(self.m):
            self.registers[i] = max(self.registers[i], other.registers[i])

    def estimate(self):
        Z = sum(2 ** -r for r in self.registers)
        E = self.alpha_m * self.m ** 2 / Z
        V = self.registers.count(0)
        if E <= 5/2 * self.m:
            return self.m * math.log(self.m / V) if V != 0 else E
        elif E <= 1/30 * (1 << 64):
            return E
        else:
            return - (1 << 64) * math.log(1 - E / (1 << 64))

    def reset(self):
        self.registers = bytearray(self.m)