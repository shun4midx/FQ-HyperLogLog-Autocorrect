############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: symspell_test.py                   #
############################################

import sys
import time
from collections import defaultdict

# ======== Edit-Distance (Wagner-Fischer) for final ranking ========= #
def levenshtein(a, b):
    n, m = len(a), len(b)
    if n < m:
        return levenshtein(b, a)
    prev = list(range(m + 1))
    for i, ca in enumerate(a, 1):
        curr = [i] + [0] * m
        for j, cb in enumerate(b, 1):
            curr[j] = min(
                curr[j - 1] + 1,            # insertion
                prev[j]     + 1,            # deletion
                prev[j - 1] + (ca != cb)    # substitution
            )
        prev = curr
    return prev[m]

# ======== Generate deletion variants up to distance k ======== #
def generate_deletes(word, k):
    deletes = {word}
    for _ in range(k):
        new_set = set()
        for w in deletes:
            for i in range(len(w)):
                new_set.add(w[:i] + w[i+1:])
        deletes |= new_set
    return deletes

# ======== Generate all single-transposition variants ======== #
def generate_transposes(word: str) -> set[str]:
    trans = set()
    for i in range(len(word)-1):
        w = list(word)
        w[i], w[i+1] = w[i+1], w[i]
        trans.add("".join(w))
    return trans

# ======== Build full SymSpell index: deletions only (efficient) ======== #
def build_symspell_dict(words: list[str], max_dist: int) -> dict[str, set[str]]:
    deletion_map = defaultdict(set)
    for w in words:
        for var in generate_deletes(w, max_dist):
            deletion_map[var].add(w)
    return deletion_map

def autocorrect_full_symspell(word_dictionary, display_map, queries, max_dist=5, output_file="outputs/20k_symspell.txt"):
    # Frequency ranking: higher freq = lower index
    N = len(word_dictionary)
    freq_map = {w: N - i for i, w in enumerate(word_dictionary)}
    word_set = set(word_dictionary)

    # 1) Build deletion map
    t0 = time.perf_counter()
    deletion_map = build_symspell_dict(word_dictionary, max_dist)
    build_time = time.perf_counter() - t0

    # 2) Query
    suggestions = []
    t1 = time.perf_counter()
    for q in queries:
        cands: set[str] = set()

        # a) Deletion-based candidates
        for var in generate_deletes(q, max_dist):
            cands |= deletion_map.get(var, set())

        # b) Transposition-based candidates
        for t in generate_transposes(q):
            if t in word_set:
                cands.add(t)

        # c) Substitution-based candidates
        for i in range(len(q)):
            prefix, suffix = q[:i], q[i+1:]
            for ch in "abcdefghijklmnopqrstuvwxyz":
                sub = prefix + ch + suffix
                if sub in word_set:
                    cands.add(sub)

        if not cands:
            suggestions.append("")
            continue

        # d) Pick best by (edit distance, -frequency)
        best = min(cands, key=lambda w: (levenshtein(q, w), -freq_map.get(w, 0)))
        suggestions.append(display_map.get(best, best))
    query_time = time.perf_counter() - t1

    # 3) Write out
    with open(output_file, "w", encoding="utf-8") as out_f:
        out_f.write("\n".join(suggestions))

    print(f"SymSpell‑Full build time: {build_time:.3f}s")
    print(f"SymSpell‑Full query time ({len(queries)} queries): {query_time:.3f}s")
    return suggestions

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python symspell_full.py <dictionary_file> <queries_file>")
        sys.exit(1)

    dict_file, query_file = sys.argv[1:]

    # Load dictionary & display map
    with open(dict_file, 'r', encoding='utf-8') as f:
        raw = [line.strip() for line in f if line.strip()]
    word_dict   = [w.lower() for w in raw]
    display_map = {w.lower(): w for w in raw}

    # Load queries
    with open(query_file, 'r', encoding='utf-8') as f:
        queries = [line.strip().lower() for line in f if line.strip()]

    autocorrect_full_symspell(word_dict, display_map, queries, max_dist=5, output_file="outputs/20k_symspell.txt")