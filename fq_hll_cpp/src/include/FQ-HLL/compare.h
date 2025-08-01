/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ Header file               *
 * File: compare.h                          *
 ****************************************** */

// ======== INCLUDE ======== //
#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// ======== FUNCTION PROTOTYPES ======== //
void compare_files(std::filesystem::path file1, std::filesystem::path file2, std::filesystem::path ground_truth);