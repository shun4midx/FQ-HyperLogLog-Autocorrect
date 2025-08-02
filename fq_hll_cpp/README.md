## Installation
### 1. Clone the Repo

```bash
git clone https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect
cd FQ-HyperLogLog-Autocorrect/fq_hll_cpp
```

Remember to **NOT** delete the repo folder if you are using the C++ FQ-HLL library, or else code may not function properly. Only do so if you plan to uninstall.

### 2. Build and Install

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
sudo cmake --install build
```

This installs:

* Headers to `/usr/local/include/FQ-HLL`
* Static library to `/usr/local/lib/libfq_hll.a`
* CMake config to `/usr/local/lib/cmake/fq_hll`
* CLI shims: `/usr/local/bin/fq_hll_g++`, `/usr/local/bin/fq_hll_clang++`

## Usage

For optimal performance, please run the code with `-O2` or `-O3`.

### Option 1: CMake Project (Recommended)

If you're using CMake, create a new file:
```cmd
touch CMakeLists.txt
```

Open it, then paste the following:

```cmake
cmake_minimum_required(VERSION 3.10)
project(<project_name>)

# Set C++17 and optimization
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)  # Ensures -O2 by default

find_package(fq_hll REQUIRED)

add_executable(<project_name> <file_name>.cpp)
target_link_libraries(<project_name> PRIVATE fq_hll::fq_hll)
```

Compile:

```bash
cmake -B build
cmake --build build
```

Execute:
```bash
./build/<project_name>
```

### Option 2: Test Projects

If you want to compile directly without setting include/lib flags:

```bash
fq_hll_g++ <file_name>.cpp -o <file_exec>
./<file_exec>
```

Or:

```bash
fq_hll_clang++ <file_name>.cpp -o demo
./<file_exec>
```

These default to `-std=c++17 -O2` and handle linking for you, where the `O2` flag ensures faster performance.


## Uninstall

To completely remove the library:

### Option 1: Manual

```bash
sudo rm -rf /usr/local/include/FQ-HLL
sudo rm -f /usr/local/lib/libfq_hll.a
sudo rm -rf /usr/local/lib/cmake/fq_hll
sudo rm -f /usr/local/bin/fq_hll_g++
sudo rm -f /usr/local/bin/fq_hll_clang++
```

### Option 2: Run Uninstall Script

```bash
cd FQ-HyperLogLog-Autocorrect/fq_hll_cpp/scripts
./uninstall_fq_hll
```

This script will prompt for confirmation and cleanly remove all installed files.

## Requirements
* CMake >= 3.10
* C++17-compatible compiler

## Notice for Beginners
It is completely normal to maybe see your compiler perhaps underline code in red related to the FQ-HLL library. This is because your IntelliSense may not look for the `FQ-HyperLogLog-Autocorrect/fq_hll_cpp` directory. However, if the code compiles fine, then it is fine.

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