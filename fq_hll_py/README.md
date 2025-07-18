## Installing the Library
First make sure you can use `pip install`. If not, please install it [here](https://pypi.org/project/pip/).

Then, either do 

```py
pip install fq_hll
```

or

```py
pip install DyslexicLogLog
```

Now, you can use the library!

## Usage
The library defaults to searching within its own folder before searching in your local directory. There are two text files offered as base dictionaries: `20k_shun4midx.txt` and `database.txt`, with around 20000 and 400 words respectively. The below code would only visit the local directory. If no dictionary is specified, `20k_shun4midx.txt` would be used instead.

```py
# ======== SAMPLE USAGE ======== #
from fq_hll import Autocorrector # Or "from dyslexicloglog import Autocorrector", just choose the one you installed

if __name__ == "__main__":
    ac = Autocorrector()

    # File
    ans1 = ac.autocorrect("test_files/typo_file.txt", "test_files/class_suggestions.txt")
    print(ans1)

    ans2 = ac.top3("test_files/typo_file.txt", "test_files/class_suggestions.txt")

    # Optionally, you can not want it to output it into a file, then:
    # Individual strings
    ans3 = ac.autocorrect("hillo")
    ans4 = ac.top3("hillo")

    # Arrays
    ans5 = ac.autocorrect(["tsetign", "hillo", "goobye", "haedhpoesn"])
    ans6 = ac.top3(["tsetign", "hillo", "goobye", "haedhpoesn"])

    # You can even have a custom dictionary!
    dictionary = ["apple", "banana", "grape", "orange"] # Note that this dictionary would be treated as if it's from most to least frequently used
    custom_ac = Autocorrector(dictionary)

    ans7 = custom_ac.autocorrect(["applle", "banana", "banan", "orenge", "grap", "pineapple"])
    ans8 = custom_ac.top3(["applle", "banana", "banan", "orenge", "grap", "pineapple"])

    print(ans7)
    print(ans8)
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

Of course, `fq_hll` can be replaced with `dyslexicloglog` here too, depending on which version you install.

## Results
In the end, dictionary list `database.txt` performed consistently at around **87~88%** accuracy and `20k_shun4midx.txt` at around **59~60%** accuracy. For the `top3` results, it was consistently at around **94~95%** and **75~76%** respectively.

For context, I implemented the standard Levenshtein + BK-Tree autocorrection algorithm with **edit distance <= 2** (Since otherwise it would be more than five times slower than FQ-HLL) in `fq_hll_py/tests/bk_test.py`, and it performs slower but also at a lower accuracy, at around **75~76%** and **43~44%** respectively. I even increased the **edit distance to be <= 3**, and allowed the program to be exponentially slower. Even then, its accuracy only achieves around **89~90%** and **46~47%** respectively, undoubtedly it uses more memory too. The accuracy doesn't increase much after edit distance is greater than 3.

Even for **`SymSpell`** in `fq_hll_py/tests/symspell_test.py`, I increased to **edit distance <= 5**, and even then its accuracy was only around **89~90%** and **46~47%** respectively.

Here is a rough total runtime of each algorithm to finish all queries:
| Method        | `database.txt`      | `20k_shun4midx.txt` |
| ------------- | ------------------- | ------------------- |
| FQ-HLL        | 0.223s              | 9.955s              |
| BK (ED <= 2)  | 2.126s              | 46.028s             |
| BK (ED <= 3)  | 3.816s              | 92.812s             |
| SymSpell      | 0.515s + 0.996s     | 16.994s + 29.355s   | <- Build + Query time

Given the relatively small memory usage yet huge accuracy and its potential to have LDP, FQ-HLL is something worth considering for autocorrection algorithms.