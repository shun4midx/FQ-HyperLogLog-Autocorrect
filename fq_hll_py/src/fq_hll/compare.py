############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: compare.py                         #
############################################

import sys

def compare_files(file1, file2, ground_truth):
    with open(file1, 'r', encoding='utf-8') as f1, \
         open(file2, 'r', encoding='utf-8') as f2, \
         open(ground_truth, 'r', encoding='utf-8') as fg:
        out1 = [line.rstrip('\n') for line in f1.readlines()]
        out2 = [line.rstrip('\n') for line in f2.readlines()]
        gold = [line.rstrip('\n') for line in fg.readlines()]

    max_len = max(len(out1), len(out2), len(gold))
    out1 += [''] * (max_len - len(out1))
    out2 += [''] * (max_len - len(out2))
    gold += [''] * (max_len - len(gold))

    correct1 = correct2 = 0

    print(f"{'Idx':<5} | {'Truth':<20} | {'File 1':<20} | {'File 2':<20} | Result")
    print("-" * 95)

    for i in range(max_len):
        t, a, b = gold[i], out1[i], out2[i]

        result = ""
        if a == t:
            correct1 += 1
            result += "OK1 "
        if b == t:
            correct2 += 1
            result += "OK2 "

        print(f"{i+1:<5} | {t:<20} | {a or '[empty]':<20} | {b or '[empty]':<20} | {result or 'X'}")

    print("\nSummary")
    print("-" * 95)
    print(f"Total queries            : {max_len}")
    print(f"Correct in File 1        : {correct1}")
    print(f"Correct in File 2        : {correct2}")
    print(f"Accuracy (File 1)        : {correct1 / max_len * 100:.2f}%")
    print(f"Accuracy (File 2)        : {correct2 / max_len * 100:.2f}%")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python compare.py suggestions.txt typos.txt true_answers.txt")
        sys.exit(1)

    compare_files(sys.argv[1], sys.argv[2], sys.argv[3])