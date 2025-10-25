# Frequency-Quantized HyperLogLog Autocorrect 
<a href="https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect/tree/main/fq_hll_cpp"><img src="https://img.shields.io/badge/c++-%23f34b7d.svg?style=for-the-badge&logo=c%2B%2B"><a href="https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect/tree/main/fq_hll_py"><img src="https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=fff&style=for-the-badge">

## NOTE: Specific details on how to install and run the programs in Python and C++ are in the `fq_hll_py` and `fq_hll_cpp` folders separately, and more information about the actual algorithm is in `algo_description/description.pdf`

## NOTE 2: More detailed documentation for the actual libraries is coming soon. Please give me time.

An improved Frequency-Quantized HyperLogLog (FQ-HLL) Autocorrection Library based on my final project in **Advanced Data Structures 2025**, with credit to my team members at the time. For respect of their privacy, I would keep them anonymous, unless they request otherwise.

## Contents
- [Context](#context)
- [Results](#results)
- [Remark on Keyboards](#remark-on-keyboards)
- [Notes](#notes)
- [Plans](#plans-for-the-repo)
- [Current Usages](#current-repos-using-this-fq-hll-library)
- [License](#license)

## Context
For context, we did Autocorrection for our final project, and I was tasked to design an Autocorrection algorithm using HLL. HLL demonstrated decent performance (accuracy, speed, and memory) compared to the [baseline](https://arxiv.org/pdf/2208.05264) with preliminary trials, but in our evaluation, we realized the way our algorithm didn't give HLL a fair chance. 

After the semester, I doubt my team members have much interest, but I still have some interest in improving it to see how HLL would perform if given a fair chance. Hence, I propose adjusting HLL's naive distinct object counter to be Frequency-Quantized, resulting in an algorithm with "FQ-HLL". I have also refactored the algorithm quite a bit when doing so, to increase accuracy.

**For theoretical value, there is nothing in my algorithm that uses any information about the English language or the QWERTY keyboard, to do any of the corrections. It is a NON-ML algorithm too.**

## Results
I've referenced [a list of common typos in datasets from Peter Norvig's classic spelling corrector](https://www.kaggle.com/datasets/bittlingmayer/spelling/data), which I should call the `typo_file.txt`, and used FQ-HLL with two different sets of base "dictionary words". As I don't have access to the correct words in `typo_file.txt`'s relative frequencies, I simply put all the corresponding words in the order as they were given, and put it in a set of dictionary words: `database.txt`. 

The other set was comprised of the [most commonly used 20000 English words](https://github.com/first20hours/google-10000-english/blob/master/20k.txt) on top, and the original words from `database.txt` on the bottom, since I needed to include all possible answers in the dictionary. I put them at the end to disadvantage them as much as possible, to be seen as least frequent. This is called `20k_shun4midx.txt`.

Here, for the most objective measure, I counted "accuracy" as simply if the word matches what the typo originally was intended to correct to. For example, if "mant" was supposed to correct to "want" in the list, even if my code outputted "many", I'd still count it wrong.

In the end, dictionary list `database.txt` performed consistently at around **87~88%** accuracy and `20k_shun4midx.txt` at around **59~60%** accuracy. For the `top3` results, it was consistently at around **93~94%** and **75~76%** respectively.

For context, I implemented the standard Levenshtein + BK-Tree autocorrection algorithm with **edit distance <= 2** (Since otherwise it would be more than five times slower than FQ-HLL) in `fq_hll_py/tests/bk_test.py`, and it performs slower but also at a lower accuracy, at around **75~76%** and **43~44%** respectively. I even increased the **edit distance to be <= 3**, and allowed the program to be slower. Even then, its accuracy only achieves around **89~90%** and **46~47%** respectively, undoubtedly it uses more memory too. The accuracy doesn't increase much after edit distance is greater than 3.

Even for **`SymSpell`** in `fq_hll_py/tests/symspell_test.py`, I increased to **edit distance <= 5**, and even then its accuracy was only around **89~90%** and **46~47%** respectively.

In `Python`, here is a rough total runtime of each algorithm to finish all queries:
| Method        | `database.txt`      | `20k_shun4midx.txt` |
| ------------- | ------------------- | ------------------- |
| FQ-HLL        | 0.223s              | 9.955s              |
| BK (ED <= 2)  | 2.126s              | 46.028s             |
| BK (ED <= 3)  | 3.816s              | 92.812s             |
| SymSpell      | 0.515s + 0.996s     | 16.994s + 29.355s   |

*(SymSpell times are Build + Query time)*

Inspired by how autocorrection on mobile devices offer top 3 suggestions, I have also implemented a top3 function. For **Top 3** results, where accuracy is counted for the number of queries that have one correct answer in the top 3 suggested results, FQ-HLL was able to reach accuracies of **93~94%** and **75~76%** for `database.txt` and `20k_shun4midx.txt` respectively. The times they took are roughly 0.308s and 14.602s respectively.

In `C++`, due to time constraints and the fact that Python demonstrated well enough the efficiency and relative accuracy of FQ-HLL, only FQ-HLL has been demonstrated here, and top3 results are as shown. Execution details are in the README file of `fq_hll_cpp`, most importantly, remember to use the `-O2` flag. The accuracy was unchanged.

| Method        | `database.txt`      | `20k_shun4midx.txt` |
| ------------- | ------------------- | ------------------- |
| FQ-HLL (Auto) | 0.071s              | 2.649s              |
| FQ-HLL (Top3) | 0.084s              | 3.716s              |

Given the relatively small memory usage yet huge accuracy and its potential to have LDP, FQ-HLL is something worth considering for autocorrection algorithms.

## Remark on Keyboards
As a side note, I made the QWERTY keyboard (including AZERTY, QWERTZ, Colemak, Dvorak, or any other custom keyboard layout) as toggleable parameters to influence my FQ-HLL, since I am coding with [Ducky](https://github.com/ducky4life) to create an FQ-HLL Android keyboard. In this case, runtime slowed down by only 1 second for the `20k_shun4midx.txt` file, but achieving accuracy of **64~65%** and **80~81%**, for the autocorrection and top 3 results respectively. However, the main takeaway of this repository is how strong FQ-HLL is without the knowledge of a keyboard layout, which is why I make it something that can be turned off, and most results would be dedicated to that.

Specific details in how these keyboards can be accessed in the programming languages are available in `fq_hll_cpp/README.md` and `fq_hll_py/README.md` separately.

## Notes
 - HLL naturally doesn't have Local Differential Privacy (LDP) yet, but has natural obfuscation.
 - This library does not collect personal data. However, still use it at your own discretion.
 
### Dyslexia
Personally, I've always had an interest in autocorrect because I'm dyslexic and often unintentionally scramble or reverse letters when I read. Here are my thoughts about this algorithm based on my dyslexia.
 - Reasoning would be more detailed in the `algo_description/description.pdf` file, but I find this algorithm's autocorrection suggestions are sometimes more intuitive (e.g. "sklof" -> "folks") to my dyslexia than other Levenshtein distance-based autocorrection models.
 - As a side note, as a dyslexic person, I naturally process words similar to how the FQ-HLL algorithm processes words, and that was my intuition in terms of how to create this algorithm in the first place.

## Plans for the Repo
 - ✅ Develop a Python library importable via `pip install`
 - ✅ Include a C++ library that is importable via CMake, since as most of you know, I love C++.
 - Formally document the logic behind the algorithm via a LaTeX file (or its PDF directly).
 - If time permits, I may include a formal proof and potential developments in LDP.

## Current Repos using this FQ-HLL Library
 <a href="https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect/tree/main/fq_hll_cpp"><img src="https://img.shields.io/badge/c++-%23f34b7d.svg?style=for-the-badge&logo=c%2B%2B">
 - `FQ-HLL Keyboard`: [An Android mobile keyboard](https://github.com/shun4midx/FQ-HLL-Keyboard) that integrates this FQ-HLL autocorrect library. It serves as a semi-official real-world use case for the algorithm alongside this specific library, and I am beyond honored to be a part of its development with [Ducky](https://github.com/ducky4life).
 - `FQ-HLL Bot`: A [C++ Discord bot](https://github.com/shun4midx/FQ-HLL-Bot) which is the semi-official real-world use case for this algorithm displayed as a Discord bot for convenience of testing.

 <a href="https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect/tree/main/fq_hll_py"><img src="https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=fff&style=for-the-badge">

 - `smortie`: A [discord.py music bot](https://github.com/ducky4life/smortie) which uses FQ-HLL autocorrect to deal with [search queries](https://github.com/ducky4life/smortie/blob/main/music.py#L144), in order to play songs.
 - `Web autocorrector`: An [autocorrector website](https://web-autocorrector.vercel.app/) which uses FQ-HLL autocorrect to deal with inputs
 - `klofr`: A [discord.py bot interface](https://github.com/ducky4life/klofr) for FQ-HLL that autocorrects every word in each message

## License
MIT License, reference `LICENSE` for more information.