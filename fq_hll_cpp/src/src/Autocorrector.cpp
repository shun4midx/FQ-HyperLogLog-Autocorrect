/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HyperLogLog-Autocorrect      *
 * File Type: C++ file                      *
 * File: Autocorrector.cpp                  *
 ****************************************** */

// ======== INCLUDE ======== //
#include "../include/FQ-HLL/Autocorrector.h"

// ======== FUNCTIONS ======== //
std::vector<std::string> extract_qgrams(std::string& word, int q, bool fuzzier) {
    if (word.length() < q) {
        return std::vector<std::string>();
    }

    std::vector<std::string> qgrams;

    for (int i = 0; i < word.length() - q + 1; ++i) {
        std::string qgram = word.substr(i, q);
        qgrams.push_back(qgram);

        if (q == 2) {
            qgrams.push_back(qgram); // Push again
            qgrams.push_back(std::string(1, qgram[0]) + " ");
            qgrams.push_back(" " + std::string(1, qgram[1]));
            
            if (fuzzier) {
                qgrams.push_back(std::string(1, qgram[1]) + std::string(1, qgram[0]));
            }
        }
    }

    return qgrams;
}

bool is_valid(std::string& word, std::unordered_set<char> letters) {
    if (letters.empty()) {
        return true;
    } else {
        return std::all_of(
            word.begin(), word.end(),
            [&](char c){
                unsigned char lc = static_cast<char>(std::tolower(c));
                return letters.find(lc) != letters.end();
            }
        );
    }
}

WordData load_words(std::vector<std::string>& arr, std::unordered_set<char> letters) {
    WordData wd;
    wd.words.reserve(arr.size());
    wd.display.reserve(arr.size());

    for (auto& raw : arr) {
        if (!is_valid(raw, letters)) {
            continue;
        }

        std::string lower;
        lower.reserve(raw.size());
        std::transform(raw.begin(), raw.end(), std::back_inserter(lower), [](unsigned char c){ return std::tolower(c); });

        wd.words.push_back(lower);
        wd.display[lower] = raw;
    }

    return wd;
}

WordData load_words(std::string& str, std::unordered_set<char> letters) { // Either is a file path or a single string input
    std::vector<std::string> raw;

    // Process
    std::filesystem::path p{str};
    if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
        std::ifstream in{p};
        if (!in) {
            throw std::runtime_error("Cannot open dictionary file: " + str);
        }
        
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) {
                raw.push_back(line);
            }
        }
    } else { // Treat the string as one word
        raw.push_back(str);
    }

    // Call
    return load_words(raw, letters);
}

WordData load_words(StrVec sv, std::unordered_set<char> letters) {
    if (auto svec = std::get_if<std::vector<std::string>>(&sv)) {
        return load_words(*svec, letters);
    } else if (auto str = std::get_if<std::string>(&sv)) {
        return load_words(*str, letters);
    } else { // Fallback
        return (WordData){};
    }
}

inline int popcount64(uint64_t x) { 
    return __builtin_popcountll(x);
}

// ======== Autocorrector CLASS: PUBLIC ======== //
Autocorrector::Autocorrector(AutocorrectorCfg& cfg) : Autocorrector(cfg.dictionary_list, cfg.valid_letters, cfg.keyboard, cfg.alpha, cfg.beta, cfg.b) {}

Autocorrector::Autocorrector(StrVec _dictionary_list, StrVec _valid_letters, StrVec _keyboard, double _alpha, double _beta, int _b) {
    // Deal with allowed letters only
    std::vector<std::string> normalized_letters = StrVecToVec(_valid_letters);
    if (!letters.empty()) {
        letters.clear();
    }

    if (normalized_letters.size() != 0) {
        for (auto& letter : normalized_letters) {
            if (letter == "a-z") {
                for (char i = 'a'; i <= 'z'; ++i) {
                    letters.insert(i);
                }
            } else if (letter == "0-9") {
                for (char i = '0'; i <= '9'; ++i) {
                    letters.insert(i);
                }
            } else if (letter.length() == 1 && letter != " ") {
                letters.insert(std::tolower(letter[0]));
            } else {
                throw std::invalid_argument("{valid_letters} should contain single character non-space letters only, other than abbreviations a-z and 0-9");
            }
        }
    }

    // Deal with keyboard
    if (!keyboard.empty()) {
        keyboard.clear();
    }

    if (auto p = std::get_if<std::string>(&_keyboard)) {
        if (*p == "qwerty") {
            keyboard = {"1234567890", "qwertyuiop", "asdfghjkl", "zxcvbnm"};
        } else if (*p == "azerty") {
            keyboard = {"1234567890", "azertyuiop", "qsdfghjklm", "wxcvbn"};
        } else if (*p == "qwertz") {
            keyboard = {"1234567890", "qwertzuiopü", "asdfghjklöä", "yxcvbnm"};
        } else if (*p == "dvorak") {
            keyboard = {"1234567890", "'  pyfgcrl", "aoeuidhtns", " qjkxbmwvz"};
        } else if (*p == "colemak") {
            keyboard = {"1234567890", "qwfpgjluy", "arstdhneio", "zxcvbkm"};
        }
    } else {
        keyboard = std::get<std::vector<std::string>>(_keyboard);
    }

    KEY_POS.clear();

    for (int i = 0; i < keyboard.size(); ++i) {
        for (int j = 0; j < keyboard[i].size(); ++j) {
            KEY_POS[keyboard[i][j]].x = i;
            KEY_POS[keyboard[i][j]].y = j;
        }
    }

    // Deal with dictionary
    std::vector<std::string> raw;
    if (auto pvec = std::get_if<std::vector<std::string>>(&_dictionary_list)) {
        raw = *pvec;
    } else if (auto pstr = std::get_if<std::string>(&_dictionary_list)) {
        const std::string& key = *pstr;
        
        // Addon files
        if (std::find(addon_files.begin(), addon_files.end(), key) != addon_files.end()) {
            std::filesystem::path base_dir = std::filesystem::path(__FILE__).parent_path().parent_path();
            std::filesystem::path def_txt = base_dir / "test_files" / "20k_shun4midx.txt";
            std::filesystem::path add_txt = base_dir / "test_files" / (key + ".txt");

            auto load_into = [&](const std::filesystem::path& p) {
                std::ifstream in{p};
                if (!in) {
                    throw std::runtime_error("Cannot open " + p.string());
                }

                std::string line;
                while (std::getline(in, line)) {
                    if (!line.empty()) {
                        raw.push_back(line);
                    }
                }
            };

            load_into(def_txt);
            load_into(add_txt);
        } else { // Interpret as path
            // If provided file path
            std::filesystem::path p{key};

            if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) { // Direct file
                std::ifstream in{p};
                if (!in) {
                    throw std::runtime_error("Cannot open " + p.string());
                }

                std::string line;
                while (std::getline(in, line)) {
                    if (!line.empty()) {
                        raw.push_back(line);
                    }
                }
            } else { // Fallback relative to source
                std::filesystem::path base_dir = std::filesystem::path(__FILE__).parent_path().parent_path();
                std::filesystem::path rel = base_dir / key;
                
                if (!(std::filesystem::exists(rel) && std::filesystem::is_regular_file(rel))) {
                    throw std::runtime_error("Dictionary file not found: " + rel.string());
                }

                std::ifstream in{rel};
                std::string line;
                while (std::getline(in, line)) {
                    if (!line.empty()) {
                        raw.push_back(line);
                    }
                }
            }
        }
    } else {
        throw std::invalid_argument("Invalid variant for dictionary_list");
    }

    // Now actually parse, lower‐case, and filter via load_words(...) helper
    WordData wd = load_words(raw, letters);
    word_dict = std::move(wd.words);
    display_map = std::move(wd.display);

    word_set.clear();
    word_set.reserve(word_dict.size());

    for (auto &w : word_dict) {
        word_set.insert(w);
    }

    alpha = _alpha;
    beta = _beta;
    b = _b;

    save_dictionary();

    if (!removed_words.empty()) {
        removed_words.clear();
    }
    compact_threshold = 0.1;
}


void Autocorrector::save_dictionary() {
    start_total = std::chrono::steady_clock::now();
    t0 = std::chrono::steady_clock::now();

    q = 2;
    cfg = SketchConfig{};
    cfg.b = b;
    
    if (!qgram_sketches.empty()) {
        qgram_sketches.clear();
    }

    WORD_COUNT = word_dict.size();
    NUM_BUCKETS = 1 << ((int)(std::ceil(std::log2((double)(WORD_COUNT)) / 2.0)));
    BUCKET_SIZE = (int)(std::ceil(WORD_COUNT / NUM_BUCKETS));

    // Build FQ-HLL per q-gram
    for (int i = 0; i < word_dict.size(); ++i) {
        std::vector<std::string> qgrams = extract_qgrams(word_dict[i], q, false);

        // For fuzzy-HLL: shift by Zipf bucket, more shift = more common
        int bucket_idx = (int)std::min(NUM_BUCKETS, (int)((i + 1) / BUCKET_SIZE) + 1);
        int shift = (int)std::min((int)(std::floor(std::log2((double)(NUM_BUCKETS) / (double)(bucket_idx)))) * 4, 64);

        for (std::string& gram : qgrams) {
            if (qgram_sketches.count(gram) == 0) {
                qgram_sketches[gram] = HyperLogLog(cfg);
            }
            qgram_sketches[gram].shifted_insert(gram + "_" + word_dict[i], shift);
        }
    }

    // Precompute dict-word q gram sets for Jaccard
    t1 = std::chrono::steady_clock::now();

    all_qgrams.clear();
    all_qgrams.reserve(qgram_sketches.size());

    for (auto& [gram, sketch] : qgram_sketches) {
        all_qgrams.push_back(gram);
    }
    std::sort(all_qgrams.begin(), all_qgrams.end());

    qgram_idx.clear();
    qgram_idx.reserve(all_qgrams.size());

    for (int i = 0; i < all_qgrams.size(); ++i) {
        qgram_idx[all_qgrams[i]] = i;
    }

    TOTAL_QGRAMS = all_qgrams.size();

    int N = TOTAL_QGRAMS;
    int blocks = (N + 63) / 64;
    word_bits.clear();
    word_bits.reserve(word_dict.size());

    for (size_t i = 0; i < word_dict.size(); ++i) {
        const auto &word = word_dict[i];

        std::vector<uint64_t> ba(blocks, 0ULL);

        auto qgrams = extract_qgrams(word_dict[i], q, false);
        for (auto &gram : qgrams) {
            auto it = qgram_idx.find(gram);

            if (it == qgram_idx.end()) {
                continue;
            }

            size_t bit = it->second;  // Which bit to set
            size_t blk = bit >> 6; // Which uint64_t
            size_t off = bit & 0x3F;  // Which bit in that word
            ba[blk] |= (1ULL << off);
        }

        word_bits.push_back(std::move(ba));
    }
}

void Autocorrector::add_dictionary(StrVec to_be_added) {
    WordData worddata = load_words(to_be_added, letters);

    std::vector<std::string> words = worddata.words;
    std::unordered_map<std::string, std::string> displays = worddata.display;
    
    std::vector<std::string> added;

    for (auto& word : words) {
        if (word_set.find(word) == word_set.end()) {
            added.push_back(word);
        } else {
            removed_words.erase(word);
        }
    }

    if (added.empty()) {
        return;
    }

    int old_exp = std::log2(NUM_BUCKETS);
    int new_exp = std::ceil(std::log2(word_dict.size() + added.size()) / 2);

    // 1) Full rebuild if exponent bumps
    if (new_exp != old_exp) {
        for (auto& add : added) {
            word_dict.push_back(add);
            word_set.insert(add);
            display_map[add] = displays[add];
        }

        save_dictionary();

        return;
    }

    // 2) Otherwise true O(1) per-word work:
    // Recomupte NUM_BUCKETS (BUCKET_SIZE stays frozen)
    NUM_BUCKETS = std::ceil((word_dict.size() + added.size()) / BUCKET_SIZE);

    // For each new word: update display_map, HLL sketches, and one bitarray
    int base = word_dict.size();

    for (int i = 0; i < added.size(); ++i) {
        word_dict.push_back(added[i]);
        word_set.insert(added[i]);
        display_map[added[i]] = displays[added[i]];

        // Qgram sketches
        std::vector<std::string> qgrams = extract_qgrams(added[i], q, false);
        int bucket_idx = (base + i + 1) / BUCKET_SIZE + 1;
        int shift = std::min((int)(std::floor(std::log2(NUM_BUCKETS / bucket_idx))) * 4, 64);

        for (auto& gram : qgrams) {
            auto [it, inserted] = qgram_sketches.try_emplace(gram, cfg);
            HyperLogLog &sketch = it->second;

            sketch.shifted_insert(gram + "_" + added[i], shift);
        }
    }

    // If new qgrams appeared, extend existing bitarrays by zeros
    std::vector<std::string> new_all;
    new_all.reserve(qgram_sketches.size());
    for (auto &kv : qgram_sketches) {
        new_all.push_back(kv.first);
    }
    std::sort(new_all.begin(), new_all.end());

    int delta = (int)new_all.size() - (int)all_qgrams.size();
    if (delta > 0) {
        int new_total  = new_all.size();
        int new_blocks = (new_total + 63) / 64;

        for (auto& ba : word_bits) {
            ba.resize(new_blocks, 0ULL);
        }

        all_qgrams = std::move(new_all);
        qgram_idx.clear();
        qgram_idx.reserve(all_qgrams.size());
        for (int i = 0; i < all_qgrams.size(); ++i) {
            qgram_idx[all_qgrams[i]] = i;
        }
        TOTAL_QGRAMS = all_qgrams.size();
    }

    // Finally append one bitarray per new word
    int blocks = (TOTAL_QGRAMS + 63) / 64;
    for (auto& w : added) {
        std::vector<uint64_t> ba(blocks, 0ULL);
    
        std::vector<std::string> qgrams = extract_qgrams(w, q, false);
        for (auto& gram : qgrams) {
            auto it = qgram_idx.find(gram);
            if (it == qgram_idx.end()) {
                continue;
            }

            size_t bit = it->second; // Which bit
            size_t blk = bit >> 6; // Bit / 64
            size_t off = bit & 0x3F; // Bit % 64
            ba[blk] |= (1ULL << off);
        }
    
        word_bits.push_back(std::move(ba));
    }
}

void Autocorrector::remove_dictionary(StrVec to_be_removed) {
    WordData tbr = load_words(to_be_removed, letters);
    std::vector<std::string> words = tbr.words;

    for (auto& word : words) {
        if (word_set.find(word) != word_set.end()) {
            removed_words.insert(word);
        }
    }

    // If too many removed, full rebuild
    size_t og_size = word_dict.size();
    if (removed_words.size() >= og_size * compact_threshold) {
        word_dict.erase(
            std::remove_if(word_dict.begin(), word_dict.end(),
              [&](auto& word){
                if (removed_words.count(word)) {
                  display_map.erase(word);
                  word_set.erase(word);
                  return true;
                }
                return false;
              }),
            word_dict.end()
          );
        
        removed_words.clear();
        save_dictionary();
    }
}

Result Autocorrector::autocorrect(const std::initializer_list<std::string> queries_list, std::filesystem::path output_file, bool use_keyboard, bool return_invalid_words, bool print_details, bool print_times) {
    return autocorrect((std::vector<std::string>)(queries_list), output_file, use_keyboard, return_invalid_words, print_details, print_times);
}

Results Autocorrector::top3(const std::initializer_list<std::string> queries_list, std::filesystem::path output_file, bool use_keyboard, bool return_invalid_words, bool print_details, bool print_times) {
    return top3((std::vector<std::string>)(queries_list), output_file, use_keyboard, return_invalid_words, print_details, print_times);
}


Result Autocorrector::autocorrect(const StrVec& queries_list, std::filesystem::path output_file, bool use_keyboard, bool return_invalid_words, bool print_details, bool print_times) {
    if (print_times) {
        save_dictionary();
    }

    WordData worddata = load_words(queries_list);
    std::vector<std::string> queries = worddata.words;
    std::unordered_map<std::string, std::string> query_displays = worddata.display;

    // 3) Process queries
    t2 = std::chrono::steady_clock::now();

    std::vector<std::string> output;
    std::unordered_map<std::string, std::string> suggestions;
    std::unordered_map<std::string, double> final_scores;

    for (auto& query : queries) {
        if (!is_valid(query)) {
            if (return_invalid_words) {
                suggestions[query_displays[query]] = query_displays[query];
                final_scores[query_displays[query]] = 0.0;
                output.push_back(query_displays[query]);
                continue;
            } else {
                suggestions[query_displays[query]] = "";
                final_scores[query_displays[query]] = 0.0;
                output.push_back("");
                continue;
            }
        }

        // Print fuzzy HLL estimates per gram
        std::vector<std::string> Q_vec = extract_qgrams(query, q, true);
        std::unordered_set<std::string> Q {Q_vec.begin(), Q_vec.end()};

        if (print_details) {
            std::cout << std::setw(12) << std::right << query << " -> qgrams: |";

            for (const auto& gram : Q) {
                auto it = qgram_sketches.find(gram);
                double est = (it != qgram_sketches.end() ? it->second.estimate() : 0.0);
                std::cout << gram << "(" << est << ")" << " |";
            }

            std::cout << "\n";
        }

        // Build query bitarray
        int blocks = (TOTAL_QGRAMS + 63) / 64;
        std::vector<uint64_t> qb(blocks, 0ULL);

        for (auto& gram : Q) {
            auto it = qgram_idx.find(gram);
            if (it == qgram_idx.end()) {
                continue;
            }

            int bit = it->second;
            int blk = bit >> 6;
            int off = bit & 0x3F;
            qb[blk] |= (1ULL << off);
        }

        int qb_count = 0;
        for (auto& block : qb) {
            qb_count += popcount64(block);
        }

        // Scan all words for candidates (intersecting grams >= 1)
        std::vector<std::pair<int, int>> cand_idxs;
        cand_idxs.reserve(word_bits.size());
        for (int idx = 0; idx < word_bits.size(); ++idx) {
            if (removed_words.find(word_dict[idx]) != removed_words.end()) {
                continue;
            }

            auto &wb = word_bits[idx];
            int inter = 0;
            for (int b = 0; b < blocks; ++b) {
                inter += popcount64(wb[b] & qb[b]);
            }

            if (inter > 0) {
                cand_idxs.emplace_back(idx, inter);
            }
        }

        if (cand_idxs.empty()) {
            if (return_invalid_words) {
                if (print_details) {
                    std::cout << "  -> no overlaps; returning original: " << query_displays[query] << std::endl;
                }
                suggestions[query_displays[query]] = query_displays[query];
                final_scores[query_displays[query]] = 0.0;
                output.push_back(query_displays[query]);
                continue;
            } else {
                if (print_details) {
                    std::cout << "  -> no overlaps; returning empty" << std::endl;
                }
                suggestions[query_displays[query]] = "";
                final_scores[query_displays[query]] = 0.0;
                output.push_back("");
                continue;
            }
        }

        // d) Compute Jaccard & Zipf score R for each candidate
        std::unordered_map<int, double> J; // Jaccards
        std::unordered_map<int, double> R; // Ranks

        for (auto& [idx, inter] : cand_idxs) {
            int word_ones = 0;
            for (uint64_t block : word_bits[idx]) {
                word_ones += popcount64(block);
            }

            double uni = qb_count + word_ones - inter;
            J[idx] = (uni != 0 ? (double)(inter) / uni : 0.0);
            R[idx] = 1.0 / ((idx + 1) / BUCKET_SIZE + 1); // same Zipf normalization as before
        }

        // e) Sweep tau to pick best
        int best_idx = -1;
        double best_score = -1.0;
        double best_tau = -1.0;

        std::vector<double> tau_cands = {0.8, 0.7, 0.6, 0.5, 0.4};
        for (double& tau : tau_cands) {
            std::vector<int> passers;
            passers.reserve(J.size());
            for (auto& [idx, jval] : J) {
                if (jval >= tau) {
                    passers.push_back(idx);
                }
            }

            if (passers.empty()) {
                continue;
            }

            for (int& idx : passers) {
                double length_penalty = 1.0 - std::pow((double)(std::abs((int)(word_dict[idx].length()) - (int)(query.length()))) / (double)(query.length()), 2);
                double norm_dist = (use_keyboard ? 1.0 / (1.0 + word_dist(query, word_dict[idx])) : 1);
                double score = (J[idx] + alpha * R[idx]) * length_penalty + norm_dist * beta + (query == word_dict[idx] ? 1 : 0);

                // std::cout << word_dict[idx] << ": " << length_penalty << ", " << word_dict[idx].length() << ", " << query.length() << std::endl;

                if (score > best_score) {
                    best_score = score;
                    best_idx = idx;
                    best_tau = tau;
                }
            }
        }

        if (best_idx == -1) { // Fallback, take highest
            for (auto& [idx, jval] : J) {
                if (jval > best_score) {
                    best_idx = idx;
                    best_score = jval;
                }
            }
            best_tau = 0.4;
        }

        std::string picked = word_dict[best_idx];
        if (print_details) {
            std::cout << std::setw(12) << std::right << query << " -> picked " << std::quoted(picked) << " at tau=" << best_tau
                << "  (J=" << J[best_idx] << ", score=" << best_score << ")" << std::endl;

            std::cout << std::string(30, '-') << std::endl;
        }

        std::string displayed_picked;
        auto it = display_map.find(picked);
        if (it != display_map.end()) {
            displayed_picked = it->second;
        } else {
            displayed_picked = picked;
        }

        suggestions[query_displays[query]] = displayed_picked;
        final_scores[query_displays[query]] = best_score;
        output.push_back(displayed_picked);
    }

    // 4) Write out
    t3 = std::chrono::steady_clock::now();

    if (output_file != std::filesystem::path("None")) {
        std::ofstream out(output_file);

        if (!out) {
            throw std::runtime_error("Failed to open output file: " + output_file.string());
        }

        for (int i = 0; i < output.size(); ++i) {
            out << output[i];
            if (i + 1 < output.size()) {
                out << "\n";
            }
        }
    }

    // 5) Output time elapsed
    end_total = std::chrono::steady_clock::now();

    if (print_times) {
        double dur_build_sketches = std::chrono::duration<double>(t1 - t0).count();
        double dur_build_bitvectors = std::chrono::duration<double>(t2 - t1).count();
        double dur_query_processing = std::chrono::duration<double>(t3 - t2).count();
        double dur_total = std::chrono::duration<double>(end_total - t0).count();

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Build sketches:    " << dur_build_sketches   << "s\n";
        std::cout << "Build bit-vectors: " << dur_build_bitvectors << "s\n";
        std::cout << "Query processing:  " << dur_query_processing << "s\n";
        std::cout << "Total autocorrect: " << dur_total            << "s\n";
    }

    // Return
    return (Result){suggestions, final_scores};
}

Results Autocorrector::top3(const StrVec& queries_list, std::filesystem::path output_file, bool use_keyboard, bool return_invalid_words, bool print_details, bool print_times) {
    if (print_times) {
        save_dictionary();
    }

    WordData worddata = load_words(queries_list);
    std::vector<std::string> queries = worddata.words;
    std::unordered_map<std::string, std::string> query_displays = worddata.display;

    // 3) Process queries
    t2 = std::chrono::steady_clock::now();

    std::vector<std::string> output;
    std::unordered_map<std::string, std::vector<std::string>> suggestions;
    std::unordered_map<std::string, std::vector<double>> final_scores;

    for (auto& query : queries) {
        if (!is_valid(query)) {
            if (return_invalid_words) {
                suggestions[query_displays[query]] = {query_displays[query], "", ""};
                final_scores[query_displays[query]] = {0.0, 0.0, 0.0};
                output.push_back(query_displays[query] + "  ");
                continue;
            } else {
                suggestions[query_displays[query]] = {"", "", ""};
                final_scores[query_displays[query]] = {0.0, 0.0, 0.0};
                output.push_back("");
                continue;
            }
        }

        // Print fuzzy HLL estimates per gram
        std::vector<std::string> Q_vec = extract_qgrams(query, q, true);
        std::unordered_set<std::string> Q {Q_vec.begin(), Q_vec.end()};

        if (print_details) {
            std::cout << std::setw(12) << std::right << query << " -> qgrams: |";

            for (const auto& gram : Q) {
                auto it = qgram_sketches.find(gram);
                double est = (it != qgram_sketches.end() ? it->second.estimate() : 0.0);
                std::cout << gram << "(" << est << ")" << " |";
            }

            std::cout << "\n";
        }

        // Build query bitarray
        int blocks = (TOTAL_QGRAMS + 63) / 64;
        std::vector<uint64_t> qb(blocks, 0ULL);

        for (auto& gram : Q) {
            auto it = qgram_idx.find(gram);
            if (it == qgram_idx.end()) {
                continue;
            }

            int bit = it->second;
            int blk = bit >> 6;
            int off = bit & 0x3F;
            qb[blk] |= (1ULL << off);
        }

        int qb_count = 0;
        for (auto& block : qb) {
            qb_count += popcount64(block);
        }

        // Scan all words for candidates (intersecting grams >= 1)
        std::vector<std::pair<int,int>> cand_idxs;
        cand_idxs.reserve(word_bits.size());
        for (int idx = 0; idx < word_bits.size(); ++idx) {
            if (removed_words.count(word_dict[idx])) {
                continue;
            }

            auto &wb = word_bits[idx];
            int inter = 0;
            for (int b = 0; b < blocks; ++b) {
                inter += popcount64(wb[b] & qb[b]);
            }

            if (inter > 0) {
                cand_idxs.emplace_back(idx, inter);
            }
        }

        if (cand_idxs.empty()) {
            if (return_invalid_words) {
                if (print_details) {
                    std::cout << "  -> no overlaps; returning original: " << query_displays[query] << "  " << std::endl;
                }
                suggestions[query_displays[query]] = {query_displays[query], "", ""};
                final_scores[query_displays[query]] = {0.0, 0.0, 0.0};
                output.push_back(query_displays[query] + "  ");
                continue;
            } else {
                if (print_details) {
                    std::cout << "  -> no overlaps; returning empty" << std::endl;
                }
                suggestions[query_displays[query]] = {"", "", ""};
                final_scores[query_displays[query]] = {0.0, 0.0, 0.0};
                output.push_back("");
                continue;
            }
        }

        // d) Compute Jaccard & Zipf score R for each candidate
        std::unordered_map<int, double> J; // Jaccards
        std::unordered_map<int, double> R; // Ranks

        for (auto& [idx, inter] : cand_idxs) {
            int word_ones = 0;
            for (uint64_t block : word_bits[idx]) {
                word_ones += popcount64(block);
            }

            double uni = qb_count + word_ones - inter;
            J[idx] = (uni != 0 ? (double)(inter) / uni : 0.0);
            R[idx] = 1.0 / ((idx + 1) / BUCKET_SIZE + 1); // same Zipf normalization as before
        }

        // e) Sweep tau to pick top 3
        // Build Jaccard + Zipf score for all candidates
        std::vector<std::pair<double, int>> scored;
        for (auto& [idx, jval] : J) {
            double length_penalty = 1.0 - std::pow((double)(std::abs((int)(word_dict[idx].length()) - (int)(query.length()))) / (double)(query.length()), 2);
            double base_score = (J[idx] + alpha * R[idx]) * length_penalty;
            scored.push_back(std::pair{base_score, idx});
        }

        // Take only the top-`shortlist` by base_score
        std::sort(scored.begin(), scored.end(),
            [](auto const &a, auto const &b){
                return a.first > b.first; // Desc
            }
        );

        size_t shortlist_size = std::min(scored.size(), size_t(30));
        std::vector<std::pair<double,int>> shortlist(scored.begin(), scored.begin() + shortlist_size);

        // If use_keyboard, recompute with keyboard distance
        std::vector<std::pair<double, int>> final;

        for (auto& [base_score, idx] : shortlist) {
            double score;
            if (use_keyboard) {
                double norm_dist = 1.0 / (1.0 + word_dist(query, word_dict[idx]));
                score = base_score + beta * norm_dist + (query == word_dict[idx] ? 1 : 0);
            } else {
                score = base_score + (query == word_dict[idx] ? 1 : 0);
            }

            final.push_back(std::pair{score, idx});
        }

        // Sort and return top 3
        std::sort(final.begin(), final.end(),
            [](auto const &a, auto const &b){
                return a.first > b.first; // Desc
            }
        );

        std::unordered_set<std::string> seen;
        std::vector<std::string> top3;
        std::vector<double> top3_scores;

        for (auto& [score, idx] : final) {
            std::string suggestion = display_map[word_dict[idx]];

            if (seen.find(suggestion) == seen.end()) {
                seen.insert(suggestion);
                top3.push_back(suggestion);
                top3_scores.push_back(score);

                if (top3.size() == 3) {
                    break;
                }
            }
        }

        while (top3.size() < 3) {
            top3.push_back("");
            top3_scores.push_back(0.0);
        }

        if (print_details) {
            std::cout << std::setw(12) << std::right << query << " -> top 3: " << top3[0] << ", " << top3[1] << ", " << top3[2] << std::endl;
            std::cout << std::string(30, '-') << std::endl;
        }

        suggestions[query_displays[query]] = top3;
        final_scores[query_displays[query]] = top3_scores;
        output.push_back(top3[0] + " " + top3[1] + " " + top3[2]);
    }

    // 4) Write out
    t3 = std::chrono::steady_clock::now();

    if (output_file != std::filesystem::path("None")) {
        std::ofstream out(output_file);

        if (!out) {
            throw std::runtime_error("Failed to open output file: " + output_file.string());
        }

        for (int i = 0; i < output.size(); ++i) {
            out << output[i];
            if (i + 1 < output.size()) {
                out << "\n";
            }
        }
    }

    // 5) Output time elapsed
    end_total = std::chrono::steady_clock::now();

    if (print_times) {
        double dur_build_sketches = std::chrono::duration<double>(t1 - t0).count();
        double dur_build_bitvectors = std::chrono::duration<double>(t2 - t1).count();
        double dur_query_processing = std::chrono::duration<double>(t3 - t2).count();
        double dur_total = std::chrono::duration<double>(end_total - t0).count();

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Build sketches:    " << dur_build_sketches   << "s\n";
        std::cout << "Build bit-vectors: " << dur_build_bitvectors << "s\n";
        std::cout << "Query processing:  " << dur_query_processing << "s\n";
        std::cout << "Total autocorrect: " << dur_total            << "s\n";
    }

    // Return
    return (Results){suggestions, final_scores};
}

// ======== Autocorrector CLASS: PRIVATE ======== //
double Autocorrector::key_dist(char& a, char& b) {
    int xa = 0, ya = 0, xb = 0, yb = 0;

    auto it_a = KEY_POS.find(a);
    if (it_a != KEY_POS.end()) {
        xa = it_a->second.x;
        ya = it_a->second.y;
    }

    auto it_b = KEY_POS.find(b);
    if (it_b != KEY_POS.end()) {
        xb = it_b->second.x;
        yb = it_b->second.y;
    }

    int dx = xa - xb;
    int dy = ya - yb;

    return std::sqrt(dx * dx + dy * dy);
}

double Autocorrector::word_dist(const std::string& a, const std::string& b) {
    int na = a.length();
    int nb = b.length();

    std::vector<double> prev(nb + 1), curr(nb + 1);

    for (int j = 0; j <= nb; ++j) {
        prev[j] = double(j);
    }

    for (int i = 1; i <= na; ++i) {
        curr[0] = double(i);
        char ai = std::tolower(a[i - 1]);
        for (int j = 1; j <= nb; ++j) {
            char bj = std::tolower(b[j - 1]);
            double cost_sub = prev[j - 1] + key_dist(ai, bj);
            double cost_del = prev[j] + 1.0;
            double cost_ins = curr[j - 1]+ 1.0;
            curr[j] = std::min({cost_sub, cost_del, cost_ins});
        }
        std::swap(prev, curr);
    }
    return prev[nb];
}

bool Autocorrector::is_valid(std::string& word) {
    return ::is_valid(word, letters);
}

std::vector<std::string> Autocorrector::StrVecToVec(StrVec sv) {
    if (auto p = std::get_if<std::string>(&sv)) {
        if (p->empty()) {
            return std::vector<std::string>{};
        } else {
            return std::vector<std::string>{*p};
        }
    } else {
        return std::get<std::vector<std::string>>(sv);
    }
}