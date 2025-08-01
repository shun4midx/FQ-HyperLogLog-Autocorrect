/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ file                      *
 * File: compare3.cpp                       *
 ****************************************** */

// ======== INCLUDE ======== //
#include "../include/FQ-HLL/compare3.h"

// ======== FUNCTION IMPLEMENTATION ======== //
std::unordered_set<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    std::unordered_set<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.insert(token);
    }

    return tokens;
}

void compare3_files(std::filesystem::path file1, std::filesystem::path file2, std::filesystem::path ground_truth) {
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
    int correct1 = 0, correct2 = 0;

    // Header
    std::cout << std::left << std::setw(5)  << "Idx" << " | " << std::setw(20) << "Truth" << " | " << std::setw(30) << "File 1 (top3)" << " | " << std::setw(20) << "File 2" << " | Result\n"
              << std::string(100, '-') << "\n";

    for (int i = 0; i < max_len; ++i) {
        auto& t = gold[i];
        auto& a = out1[i];
        auto& b = out2[i];

        std::unordered_set<std::string> suggestions1 = split(a);
        std::unordered_set<std::string> suggestions2 = split(b);
        std::vector<std::string> result;

        if (suggestions1.find(t) != suggestions1.end()) {
            ++correct1;
            result.push_back("OK1");
        }

        if (suggestions2.find(t) != suggestions2.end()) {
            ++correct2;
            result.push_back("OK2");
        }

        auto disp = [&](const std::string& s){
            return s.empty() ? "[empty]" : s;
        };

        std::string joined_result = "";

        for (std::string& str : result) {
            if (!joined_result.empty()) {
                joined_result += " ";
            }
            joined_result += str;
        }

        std::cout << std::setw(5) << (i + 1) << " | " << std::setw(20) << disp(t) << " | " << std::setw(50) << (!a.empty() ? a : "[empty]") << " | " << std::setw(20) << (!b.empty() ? b : "[empty]") << " | " << (!joined_result.empty() ? joined_result : "X") << "\n";
    }

    // Summary
    std::cout << "\nSummary\n" << std::string(100, '-') << "\n"
              << std::fixed << std::setprecision(2)
              << "Total queries            : " << max_len << "\n"
              << "Correct in File 1        : " << correct1 << " (" << (double)correct1 / max_len * 100 << "%)\n"
              << "Correct in File 2        : " << correct2 << " (" << (double)correct2 / max_len * 100 << "%)\n";
}