############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: bk_test.py                         #
############################################

import sys
import time

# Simple Levenshtein distance (iterative DP)
def levenshtein(a, b):
    n, m = len(a), len(b)
    if n < m:
        return levenshtein(b, a)
    # now n >= m
    prev = list(range(m + 1))
    for i, ca in enumerate(a, 1):
        curr = [i] + [0] * m
        for j, cb in enumerate(b, 1):
            insert = curr[j - 1] + 1
            delete = prev[j] + 1
            replace = prev[j - 1] + (ca != cb)
            curr[j] = min(insert, delete, replace)
        prev = curr
    return prev[m]

# BK‑Tree node
class BKNode:
    def __init__(self, word):
        self.word = word
        self.children = {}  # distance -> BKNode

    def insert(self, word):
        d = levenshtein(word, self.word)
        if d in self.children:
            self.children[d].insert(word)
        else:
            self.children[d] = BKNode(word)

    def query(self, target, tol, collector):
        d = levenshtein(target, self.word)
        if d <= tol:
            collector.append(self.word)
        # explore children within [d - tol, d + tol]
        for dist in range(max(1, d - tol), d + tol + 1):
            child = self.children.get(dist)
            if child:
                child.query(target, tol, collector)

# Build BK‑Tree from dictionary
def build_bk_tree(words):
    it = iter(words)
    root = BKNode(next(it))
    for w in it:
        root.insert(w)
    return root

def autocorrect_bk(word_dictionary, display_map, queries, max_dist=2, output_file=f"outputs/20k_suggestions_bk_ed_2.txt"):
    # Build index map for tie-breaking by frequency
    index_map = {w: i for i, w in enumerate(word_dictionary)}
    # Build BK‑Tree
    bk = build_bk_tree(word_dictionary)

    suggestions = []
    start = time.perf_counter()
    for q in queries:
        cands = []
        bk.query(q, max_dist, cands)
        if not cands:
            suggestions.append("")
        else:
            # pick smallest (distance, frequency rank)
            best = min(
                cands,
                key=lambda w: (levenshtein(q, w), index_map[w])
            )
            suggestions.append(display_map[best])
    elapsed = time.perf_counter() - start

    # write out
    with open(output_file, "w", encoding="utf-8") as out:
        out.write("\n".join(suggestions))

    print(f"BK‑Tree autocorrect completed in {elapsed:.3f}s")
    return suggestions

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python bk_autocorrect.py <dictionary_file> <queries_file>")
        sys.exit(1)

    dict_file, query_file = sys.argv[1], sys.argv[2]
    # load words
    with open(dict_file, 'r', encoding='utf-8') as f:
        raw = [line.strip() for line in f if line.strip()]

    word_dict = [word.lower() for word in raw]
    display_map = {word.lower() : word for word in raw}
    # load queries
    with open(query_file, 'r', encoding='utf-8') as f:
        queries = [line.strip().lower() for line in f if line.strip()]

    autocorrect_bk(word_dict, display_map, queries, max_dist=2, output_file=f"outputs/20k_suggestions_bk_ed_2.txt")