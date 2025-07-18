# Frequency-Quantized HyperLogLog Autocorrect

## NOTE: Specific details on how to run the programs in Python and C++ are in the `fq_hll_py` and `fq_hll_cpp` folders separately

An improved Frequency-Quantized HyperLogLog (FQ-HLL) Autocorrection Library based on my final project in **Advanced Data Structures 2025**, with credit to my team members at the time. For their respect of privacy, I would keep them anonymous, unless they request otherwise.

## Contents
- [Context](#context)
- [Results](#results)
- [Notes](#notes)
- [Plans](#plans-for-the-repo)

## Context
For context, we did Autocorrection for our final project, and I was tasked to design an Autocorrection algorithm using HLL. HLL demonstrated decent performance (accuracy, speed, and memory) compared to the [baseline](https://arxiv.org/pdf/2208.05264) with preliminary trials, but in our evaluation, we realized the way our algorithm didn't give HLL a fair chance. 

After the semester, I doubt my team members have much interest, but I still have some interest in improving it to see how HLL would perform if given a fair chance. Hence, I propose adjusting HLL's naive distinct object counter to be Frequency-Quantized, resulting in an algorithm with "FQ-HLL". I have also refactored the algorithm quite a bit when doing so, to increase accuracy.

**For theoretical value, there is nothing in my algorithm that uses any information about the English language or the QWERTY keyboard, to do any of the corrections. It is a NON-ML algorithm too.**

## Results (For runtime/memory specific results, visit the README files of the desired language please)
I've referenced [a list of common typos in datasets from Peter Norvig's classic spelling corrector] (https://www.kaggle.com/datasets/bittlingmayer/spelling/data), which I should call the `typo_file.txt`, and used FQ-HLL with two different sets of base "dictionary words". As I don't have access to the correct words in `typo_file.txt`'s relative frequencies, I simply put all the corresponding words in the order as they were given, and put it in a set of dictionary words: `database.txt`. 

The other set was comprised of the [most commonly used 20000 English words](https://github.com/first20hours/google-10000-english/blob/master/20k.txt) on top, and the original words from `database.txt` on the bottom, since I needed to include all possible answers in the dictionary. I put them at the end to disadvantage them as much as possible, to be seen as least frequent. This is called `20k_shun4midx.txt`.

Here, for the most objective measure, I counted "accuracy" as simply if the word matches what the typo originally was intended to correct to. For example, if "mant" was supposed to correct to "want" in the list, even if my code outputted "many", I'd still count it wrong.

In the end, dictionary list `database.txt` performed consistently at around **87~88%** accuracy and `20k_shun4midx.txt` at around **59~60%** accuracy. For the `top3` results, it was consistently at around **94~95%** and **75~76%** respectively.

For context, I implemented the standard Levenshtein + BK-Tree autocorrection algorithm with **edit distance <= 2** (Since otherwise it would be more than five times slower than FQ-HLL) in `fq_hll_py/tests/bk_test.py`, and it performs slower but also at a lower accuracy, at around **75~76%** and **43~44%** respectively. I even increased the **edit distance to be <= 3**, and allowed the program to be exponentially slower. Even then, its accuracy only achieves around **89~90%** and **46~47%** respectively, undoubtedly it uses more memory too. The accuracy doesn't increase much after edit distance is greater than 3.

Even for **`SymSpell`** in `fq_hll_py/tests/symspell_test.py`, I increased to **edit distance <= 5**, and even then its accuracy was only around **89~90%** and **46~47%** respectively. 

Given the relatively small memory usage yet huge accuracy and its potential to have LDP, FQ-HLL is something worth considering for autocorrection algorithms.

## Notes
 - HLL naturally doesn't have Local Differential Privacy (LDP) yet, but has natural obfuscation.
 - This library does not collect personal data. However, still use it at your own discretion.
 
### Dyslexia
Personally, I've always had an interest in autocorrect because I'm dyslexic and often unintentionally scramble or reverse letters when I read. Here are my thoughts about this algorithm based on my dyslexia.
 - Reasoning would be more detailed in the `algo_description/description.pdf` file, but I find this algorithm's autocorrection suggestions are sometimes more intuitive (e.g. "klof" -> "folk") to my dyslexia than other Levenshtein distance-based autocorrection models.
 - As a side note, as a dyslexic person, I naturally process words similar to how the FQ-HLL algorithm processes words, and that was my intuition in terms of how to create this algorithm in the first place.

## Plans for the Repo
 - Develop a Python library importable via `pip install`
 - [Maybe] Include a C++ library too that is importable via CMake, since as most of you know, I love C++.
 - Formally document the logic behind the algorithm via a LaTeX file (or its PDF directly).
 - If time permits, I may include a formal proof of FQ-HLL, the FQ-HLL algorithm's validity, and potential developments in LDP.