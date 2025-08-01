/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ file                      *
 * File: compare.cpp                        *
 ****************************************** */

// ======== INCLUDE ======== //
#include "../include/FQ-HLL/compare.h"

// ======== FUNCTION IMPLEMENTATION ======== //
void compare_files(std::filesystem::path file1, std::filesystem::path file2, std::filesystem::path ground_truth) {
    // Read all three files into string vectors
    auto read_lines = [](const std::filesystem::path& p){
        std::ifstream in(p);
        if (!in) {
            throw std::runtime_error("Unable to open " + p.string());
        }

        std::vector<std::string> lines;
        std::string line;

        while (std::getline(in, line)) {
            // rstrip '\r' if present (for Windows CRLF)
            if (!line.empty() && line.back() == '\r') 
                line.pop_back();
            lines.push_back(line);
        }

        return lines;
    };

    std::vector<std::string> out1 = read_lines(file1);
    std::vector<std::string> out2 = read_lines(file2);
    std::vector<std::string> gold = read_lines(ground_truth);

    // Pad all three to the same length
    int max_len = std::max({out1.size(), out2.size(), gold.size()});
    out1.resize(max_len, "");
    out2.resize(max_len, "");
    gold.resize(max_len, "");

    // Compare
    int correct1 = 0, correct2 = 0, diff = 0;

    // Header
    std::cout << std::left << std::setw(5)  << "Idx" << " | " << std::setw(20) << "Truth" << " | " << std::setw(20) << "File 1" << " | " << std::setw(20) << "File 2" << " | Result\n"
              << std::string(95, '-') << "\n";

    for (int i = 0; i < max_len; ++i) {
        auto& t = gold[i];
        auto& a = out1[i];
        auto& b = out2[i];

        std::string result;
        if (a == t) {
            ++correct1;
            result += "OK1 ";
        }

        if (b == t) {
            ++correct2;
            result += "OK2 ";
        } else if (a != b) {
            ++diff;
        }

        auto disp = [&](const std::string& s){
            return s.empty() ? "[empty]" : s;
        };

        std::cout << std::setw(5) << (i + 1) << " | " << std::setw(20) << disp(t) << " | " << std::setw(20) << disp(a) << " | " << std::setw(20) << disp(b) << " | " << (result.empty() ? "X" : result) << "\n";
    }

    // Summary
    std::cout << "\nSummary\n" << std::string(95, '-') << "\n"
              << std::fixed << std::setprecision(2)
              << "Total queries            : " << max_len << "\n"
              << "Correct in File 1        : " << correct1 << " (" << (double)correct1 / max_len * 100 << "%)\n"
              << "Correct in File 2        : " << correct2 << " (" << (double)correct2 / max_len * 100 << "%)\n"
              << "Different Suggestions    : " << diff << "\n";
}