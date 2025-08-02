# FQ-HLL
An improved Frequency-Quantized HyperLogLog (FQ-HLL) Autocorrection Library

**For theoretical value, there is nothing in my algorithm that uses any information about the English language or the QWERTY keyboard, to do any of the corrections. It is a NON-ML algorithm too.**

## Usage
The library defaults to searching within its own folder before searching in your local directory. Visit my [GitHub Repo](https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect) for more details, but there are two text files offered as base dictionaries: `20k_shun4midx.txt` and `database.txt`, with around 20000 and 400 words respectively. The below code would only visit the local directory. If no dictionary is specified, `20k_shun4midx.txt` would be used instead.

What is returned is in the form of a dictionary, mapping each query to either a single string for `autocorrect` or a list of three strings for `top3`. 

```py
# ======== SAMPLE USAGE ======== #
from fq_hll import Autocorrector

if __name__ == "__main__":
    ac = Autocorrector()

    # File
    ans1 = ac.autocorrect("test_files/typo_file.txt", "outputs/class_suggestions.txt")
    print(ans1.suggestions)
    print(ans1.scores)

    ans2 = ac.top3("test_files/typo_file.txt", "outputs/class_suggestions.txt")

    # Optionally, you can not want it to output it into a file, then:
    # Individual strings
    ans3 = ac.autocorrect("hillo")
    ans4 = ac.top3("hillo")

    # Arrays
    ans5 = ac.autocorrect(["tsetign", "hillo", "goobye", "haedhpoesn"])
    ans6 = ac.top3(["tsetign", "hillo", "goobye", "haedhpoesn"])

    # You can even have a custom dictionary!
    dictionary = ["apple", "banana", "grape", "orange"]
    custom_ac = Autocorrector(dictionary)

    ans7 = custom_ac.autocorrect(["applle", "banana", "banan", "orenge", "grap", "pineapple"])
    ans8 = custom_ac.top3(["applle", "banana", "banan", "orenge", "grap", "pineapple"])

    print(ans7.suggestions)
    print(ans8.suggestions)
```

As a side note, `compare.py` and `compare3.py`, as [files](https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect/tree/main/fq_hll_py/src/fq_hll) that are quite useful for comparing between intended outputs and actual outputs, can be used via 

```py
from fq_hll import compare
compare_files(suggestions, typos, answers)
```

or if we are doing Top 3 words selected per row,

```py
from fq_hll import compare3
compare3_files(suggestions, typos, answers)
```

## Results
More detail can be found in my [GitHub Repo](https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect).

TL;DR it is way more accurate than traditional non-ML, non-language-specific algorithms such as BK-Tree and SymSpell and it also quite fast.

## Remark on Keyboards
As a side note, I made the QWERTY keyboard (including AZERTY, QWERTZ, Colemak, Dvorak, or any other custom keyboard layout) as toggleable parameters to influence my FQ-HLL, since I am coding with [Ducky](https://github.com/ducky4life) to create an FQ-HLL Android keyboard. In this case, runtime slowed down by only 1 second for the `20k_shun4midx.txt` file, but achieving accuracy of **64~65%** and **80~81%**, for the autocorrection and top 3 results respectively. However, the main takeaway of this repository is how strong FQ-HLL is without the knowledge of a keyboard layout, which is why I make it something that can be turned off, and most results woud be dedicated to that.

Notice, these keyboards are accessible in `Python` for example via:

```py
ac = Autocorrection(keyboard="qwerty")
```

or

```py
ac = Autocorrection(keyboard=["custom_row1", "custom_row2", etc])
```

## Notes
 - HLL naturally doesn't have Local Differential Privacy (LDP) yet, but has natural obfuscation.
 - This library does not collect personal data. However, still use it at your own discretion.
 
## Dyslexia
Personally, I've always had an interest in autocorrect because I'm dyslexic and often unintentionally scramble or reverse letters when I read. Here are my thoughts about this algorithm based on my dyslexia.
 - Reasoning would be more detailed in the `algo_description/description.pdf` file in my [GitHub Repo](https://github.com/shun4midx/FQ-HyperLogLog-Autocorrect), but I find this algorithm's autocorrection suggestions are sometimes more intuitive (e.g. "sklof" -> "folks") to my dyslexia than other Levenshtein distance-based autocorrection models.
 - As a side note, as a dyslexic person, I naturally process words similar to how the FQ-HLL algorithm processes words, and that was my intuition in terms of how to create this algorithm in the first place.