/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ Header file               *
 * File: Autocorrector.h                    *
 ****************************************** */

// ======== INCLUDE ======== //
#pragma once
#include "HyperLogLog.h"
#include <iostream>
#include <unordered_map>
#include <variant>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <chrono>

// ======== DEFINE ======== //
using StrVec = std::variant<std::string, std::vector<std::string>>;
static const std::vector<std::string> addon_files = {"texting"};

// ======== STRUCT ======== //
typedef struct AutocorrectorCfg {
    StrVec dictionary_list = std::filesystem::path{"test_files"} / "20k_shun4midx.txt";
    StrVec valid_letters = "a-z";
    StrVec keyboard = "qwerty";
    double alpha = 0.2;
    double beta = 0.35;
    int b = 10;
} AutocorrectorCfg;

typedef struct WordData {
    std::vector<std::string> words;
    std::unordered_map<std::string, std::string> display;
} WordData;

typedef struct ThreeResults {
    std::unordered_map<std::string, std::vector<std::string>> suggestions;
    std::unordered_map<std::string, std::vector<double>> scores;
} Results;

typedef struct SingleResult {
    std::unordered_map<std::string, std::string> suggestions;
    std::unordered_map<std::string, double> scores;
} Result;

struct Coord {
    int x;
    int y;
};

// ======== FUNCTION PROTOTYPES ======== //
std::vector<std::string> extract_qgrams(std::string& word, int q = 2, bool fuzzier = false);

bool is_valid(std::string& word, std::unordered_set<char> letters = {});
WordData load_words(std::vector<std::string>& arr, std::unordered_set<char> letters = {});
WordData load_words(std::string& str, std::unordered_set<char> letters = {}); // Either is a file path or a single string input
WordData load_words(StrVec sv, std::unordered_set<char> letters = {});
inline int popcount64(uint64_t x);

// ======== CLASS ======== //
class Autocorrector {
public:
    explicit Autocorrector(AutocorrectorCfg& cfg);
    Autocorrector& operator=(const Autocorrector& ac) = default;
    explicit Autocorrector(StrVec _dictionary_list = std::filesystem::path{"test_files"} / std::filesystem::path{"20k_shun4midx.txt"}, StrVec _valid_letters = "a-z", StrVec _keyboard = "qwerty", double _alpha = 0.2, double _beta = 0.35, int _b = 10);

    void save_dictionary();
    void add_dictionary(StrVec to_be_added);
    void remove_dictionary(StrVec to_be_removed);

    Result autocorrect(const std::initializer_list<std::string> queries_list, std::filesystem::path output_file = "None", bool use_keyboard = true, bool return_invalid_words = true, bool print_details = false, bool print_times = false);
    Results top3(const std::initializer_list<std::string> queries_list, std::filesystem::path output_file = "None", bool use_keyboard = true, bool return_invalid_words = true, bool print_details = false, bool print_times = false);

    Result autocorrect(const StrVec& queries_list, std::filesystem::path output_file = "None", bool use_keyboard = true, bool return_invalid_words = true, bool print_details = false, bool print_times = false);
    Results top3(const StrVec& queries_list, std::filesystem::path output_file = "None", bool use_keyboard = true, bool return_invalid_words = true, bool print_details = false, bool print_times = false);

private:
    // ~~~~~~~~ VARIABLES ~~~~~~~~ //
    std::unordered_set<char> letters;
    std::vector<std::string> keyboard;
    std::unordered_map<char, struct Coord> KEY_POS;

    std::vector<std::string> word_dict;
    std::unordered_set<std::string> word_set;
    std::unordered_map<std::string, std::string> display_map;

    double alpha;
    double beta;
    int b;

    std::unordered_set<std::string> removed_words;
    double compact_threshold = 0.1;

    std::chrono::steady_clock::time_point start_total, t0, t1, t2, t3, end_total;

    int q;
    SketchConfig cfg;
    std::unordered_map<std::string, HyperLogLog> qgram_sketches; // Qgram to HLL

    int WORD_COUNT;
    int NUM_BUCKETS;
    int BUCKET_SIZE;

    std::vector<std::string> all_qgrams;
    std::unordered_map<std::string, int> qgram_idx; // Qgram to idx
    int TOTAL_QGRAMS;

    std::vector<std::vector<uint64_t>> word_bits;

    // ~~~~~~~~ FUNCTIONS ~~~~~~~~ //
    double key_dist(char& a, char& b);
    double word_dist(const std::string& a, const std::string& b);
    bool is_valid(std::string& word);
    std::vector<std::string> StrVecToVec(StrVec sv);
};