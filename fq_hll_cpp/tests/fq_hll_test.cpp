/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ file                      *
 * File: fq_hll_test.cpp                    *
 ****************************************** */

#include <FQ-HLL/FQ-HLL.h>
#include <iostream>
#include <string>

int main() {
    // ======== 20k_shun4midx.txt ======== //
    AutocorrectorCfg cfg;
    cfg.valid_letters = "";

    Autocorrector ac = Autocorrector(cfg);

    // Test files
    Result autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/20k_autocorrect_suggestions.txt", false, false, false, true); // Don't use keyboard, print details and print times
           autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/qwerty_20k_autocorrect_suggestions.txt", true, false, false, true); // Use keyboard, don't print details but print times
    // std::unordered_map<std::string, std::string> sug = autocor.suggestions;
    // std::unordered_map<std::string, double> score = autocor.scores;

    Results top3_ans = ac.top3("test_files/typo_file.txt", "outputs/20k_top3_suggestions.txt", false, false, false, true); // Don't use keyboard, print details and print times
            top3_ans = ac.top3("test_files/typo_file.txt", "outputs/qwerty_20k_top3_suggestions.txt", true, false, false, true); // Use keyboard, don't print details but print times
    // std::unordered_map<std::string, std::string> sug = top3_ans.suggestions;
    // std::unordered_map<std::string, double> score = top3_ans.scores;

    compare_files("outputs/qwerty_20k_autocorrect_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt");
    compare3_files("outputs/20k_top3_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt");

    // ======== database.txt ======== //
    cfg.dictionary_list = "test_files/database.txt";
    ac = Autocorrector(cfg);

    // Test files
    autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/database_autocorrect_suggestions.txt", false, false, false, true); // Don't use keyboard, print details and print times
    autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/qwerty_database_autocorrect_suggestions.txt", true, false, false, true); // Use keyboard, don't print details but print times
    // std::unordered_map<std::string, std::string> sug = autocor.suggestions;
    // std::unordered_map<std::string, double> score = autocor.scores;

    top3_ans = ac.top3("test_files/typo_file.txt", "outputs/database_top3_suggestions.txt", false, false, false, true); // Don't use keyboard, print details and print times
    top3_ans = ac.top3("test_files/typo_file.txt", "outputs/qwerty_database_top3_suggestions.txt", true, false, false, true); // Use keyboard, don't print details but print times
    // std::unordered_map<std::string, std::string> sug = top3_ans.suggestions;
    // std::unordered_map<std::string, double> score = top3_ans.scores;

    compare_files("outputs/qwerty_database_autocorrect_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt");
    compare3_files("outputs/database_top3_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt");

    return 0;
}