## Installing the Library
First make sure you can use `pip install`. If not, please install it [here](https://pypi.org/project/pip/).

Then, either do 

```cmd
pip install fq-hll
```

or

```cmd
pip install DyslexicLogLog
```

Now, you can use the library!

If you ever want to uninstall, feel free to use `pip uninstall` as the prefix.

## Usage
The library defaults to searching within its own folder before searching in your local directory. There are two text files offered as base dictionaries: `20k_shun4midx.txt` and `database.txt`, with around 20000 and 400 words respectively. The below code would only visit the local directory. If no dictionary is specified, `20k_shun4midx.txt` would be used instead.

What is returned is in the form of a dictionary, mapping each query to either a single string for `autocorrect` or a list of three strings for `top3`. 

```py
# ======== SAMPLE USAGE ======== #
from fq_hll import Autocorrector # Or "from dyslexicloglog import Autocorrector", just choose the one you installed

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

There is also a mode for texting, which combines the `texting.txt` file here underneath the `20k_shun4midx.txt` file, when ranked according to frequency. You could simply call the following command instead of simply `ac = Autocorrector()` to use this combined dictionary:

```cmd
ac = Autocorrector("texting")
```

Although I would **NOT modify** the `20k_shun4midx.txt` file, if you use the `texting.txt` file and notice some words you commonly use when texting are missing and you want to include it, feel free to contact me (via [Email](mailto:shun4midx@gmail.com) or Discord at @shun4midx) and I will consider including it in the file. For context, words like "lol" and "omg" are already in the original `20k_shun4midx.txt` file, so please check if it is in the `20k_shun4midx.txt` file before contacting me.

The words in the `texting.txt` file are not compiled from any online source. They simply are based on commonly used texting words I observe from personally texting my friends, so they may be more biased to match my demographic. 

If you have any suggestions of other categories of words to add other than texting, feel free to let me know. I may consider creating the category to be just as usable as the texting file.

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