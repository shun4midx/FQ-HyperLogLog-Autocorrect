Full documentation and CMake files would be provided later, but please remember to use the `-O2` flag when running the code, it would make it way faster!

As of right now, without CMake, this is how the file `fq_hll_test.cpp` is run after cloning this repo:

```cmd
> g++ -std=c++17 -O2 -I../src/include -I../src/include/FQ-HLL ../src/src/*.cpp fq_hll_test.cpp
> ./a.out
```

## Usage
The library defaults to searching within its own folder before searching in your local directory. There are two text files offered as base dictionaries: `20k_shun4midx.txt` and `database.txt`, with around 20000 and 400 words respectively. The below code would only visit the local directory. If no dictionary is specified, `20k_shun4midx.txt` would be used instead.

What is returned is in the form of a dictionary, mapping each query to either a single string for `autocorrect` or a list of three strings for `top3`. 

```cpp
// ======== AS OF RIGHT NOW WITHOUT CMAKE ======== //
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

    // Vectors (Note that you would need to parse it specifically as a std::vector<std::string>)
    std::vector<std::string> vec = {"tsetign", "hillo", "goobye", "haedhpoesn"};
    Result ans5 = ac.autocorrect(vec); // Optionally, ac.autocorrect(std::vector<std::string>{"tsetign", "hillo", "goobye", "haedhpoesn"}). Yes, std::vector<std::string> here is mandatory
    Results ans6 = ac.top3(vec);

    // You can even have a custom dictionary!
    std::vector<std::string> dictionary = {"apple", "banana", "grape", "orange"};
    AutocorrectorCfg cfg;
    cfg.dictionary_list = dictionary;
    Autocorrector custom_ac(cfg);

    std::vector<std::string> inputs = {"applle", "banana", "banan", "orenge", "grap", "pineapple"};
    Result ans7 = custom_ac.autocorrect(inputs);
    Results ans8 = custom_ac.top3(inputs);

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
```

There is also a mode for texting, which combines the `texting.txt` file here underneath the `20k_shun4midx.txt` file, when ranked according to frequency. You could simply call the following command instead of simply `ac = Autocorrector()` to use this combined dictionary:

```cmd
AutocorrectorCfg cfg;
cfg.dictionary_list = "texting"
Autocorrector ac = Autocorrector(cfg)
```

Although I would **NOT modify** the `20k_shun4midx.txt` file, if you use the `texting.txt` file and notice some words you commonly use when texting are missing and you want to include it, feel free to contact me (via [Email](mailto:shun4midx@gmail.com) or Discord at @shun4midx) and I will consider including it in the file. For context, words like "lol" and "omg" are already in the original `20k_shun4midx.txt` file, so please check if it is in the `20k_shun4midx.txt` file before contacting me.

The words in the `texting.txt` file are not compiled from any online source. They simply are based on commonly used texting words I observe from personally texting my friends, so they may be more biased to match my demographic. 

If you have any suggestions of other categories of words to add other than texting, feel free to let me know. I may consider creating the category to be just as usable as the texting file.

As a side note, `compare.py` and `compare3.py`, as [files](https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect/tree/main/fq_hll_cpp/src/fq_hll) that are quite useful for comparing between intended outputs and actual outputs, can be used via 

```cpp
// ======== AS OF RIGHT NOW WITHOUT CMAKE ======== //
#include <FQ-HLL/FQ-HLL.h>
compare_files(suggestions, typos, answers)
```

or if we are doing Top 3 words selected per row,

```cpp
// ======== AS OF RIGHT NOW WITHOUT CMAKE ======== //
#include <FQ-HLL/FQ-HLL.h>
compare3_files(suggestions, typos, answers)
```

## Results
In the end, dictionary list `database.txt` performed consistently at around **87~88%** accuracy and `20k_shun4midx.txt` at around **59~60%** accuracy. For the `top3` results, it was consistently at around **93~94%** and **75~76%** respectively.

In `C++`, due to time constraints and the fact that Python [demonstrated well enough](https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect/blob/main/fq_hll_py/README.md) the efficiency and relative accuracy of FQ-HLL, only FQ-HLL has been demonstrated here, and top3 results are as shown. Execution details are in the README file of `fq_hll_cpp`, most importantly, remember to use the `-O2` flag. The accuracy was unchanged.

| Method        | `database.txt`      | `20k_shun4midx.txt` |
| ------------- | ------------------- | ------------------- |
| FQ-HLL (Auto) | 0.071s              | 2.649s              |
| FQ-HLL (Top3) | 0.084s              | 3.716s              |

Given the relatively small memory usage yet huge accuracy and its potential to have LDP, FQ-HLL is something worth considering for autocorrection algorithms.

## Remark on Keyboards
As a side note, I made the QWERTY keyboard (including AZERTY, QWERTZ, Colemak, Dvorak, or any other custom keyboard layout) as toggleable parameters to influence my FQ-HLL, since I am coding with [Ducky](https://github.com/ducky4life) to create an FQ-HLL Android keyboard. In this case, runtime slowed down by only 1 second for the `20k_shun4midx.txt` file, but achieving accuracy of **64~65%** and **80~81%**, for the autocorrection and top 3 results respectively. However, the main takeaway of this repository is how strong FQ-HLL is without the knowledge of a keyboard layout, which is why I make it something that can be turned off, and most results woud be dedicated to that.

Notice, these keyboards are accessible in `C++` for example via:

```cpp
AutocorrectorCfg cfg;
cfg.keyboard = "qwerty"
Autocorrector ac = Autocorrection(cfg)
```

or

```cpp
AutocorrectorCfg cfg;
cfg.keyboard = std::vector<std::string>{"custom_row1", "custom_row2", etc};
Autocorrector ac = Autocorrection(cfg)
```