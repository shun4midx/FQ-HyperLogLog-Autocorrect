/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ file                      *
 * File: sample_code.cpp                    *
 ****************************************** */

#include <FQ-HLL/FQ-HLL.h>
#include <iostream>
#include <string>

int main() {
    Autocorrector ac;

    // File
    Result ans1 = ac.autocorrect("test_files/typo_file.txt", "test_files/class_suggestions.txt");
    // std::unordered_map<std::string, std::string> sug = ans1.suggestions;
    // std::unordered_map<std::string, double> score = ans1.scores;

    Results ans2 = ac.top3("test_files/typo_file.txt", "test_files/class3_suggestions.txt");

    // Optionally, you can not want it to output it into a file, then:
    // Individual strings
    Result ans3 = ac.autocorrect("hillo");
    Results ans4 = ac.top3("hillo");

    // Vectors (either ans5 or ans6 parsing works)
    std::vector<std::string> vec = {"tsetign", "hillo", "goobye", "haedhpoesn"};
    Result ans5 = ac.autocorrect({"tsetign", "hillo", "goobye", "haedhpoesn"});
    Results ans6 = ac.top3(vec);

    // You can even have a custom dictionary!
    std::vector<std::string> dictionary = {"apple", "banana", "grape", "orange"};
    AutocorrectorCfg cfg;
    cfg.dictionary_list = dictionary;
    Autocorrector custom_ac(cfg);

    // Either ans7 or ans8 parsing works
    std::vector<std::string> inputs = {"applle", "banana", "banan", "orenge", "grap", "pineapple"};
    Result ans7 = custom_ac.autocorrect(inputs);
    Results ans8 = custom_ac.top3({"applle", "banana", "banan", "orenge", "grap", "pineapple"});

    for (const auto& [key, value] : ans7.suggestions) {
        std::cout << key << " -> " << value << std::endl;
    }

    for (const auto& [key, values] : ans8.suggestions) {
        std::cout << key << " -> ";

        for (int i = 0; i < 3; ++i) {
            std::cout << values[i] << "(" << ans8.scores[key][i] << ")" << " ";
        }

        std::cout << std::endl;
    }

    return 0;
}