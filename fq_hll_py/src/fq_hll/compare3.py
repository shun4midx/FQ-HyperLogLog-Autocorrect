############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: compare3.py                        #
############################################

import sys

def compare3_files(file1, file2, ground_truth):
    with open(file1, 'r', encoding='utf-8') as f1, \
         open(file2, 'r', encoding='utf-8') as f2, \
         open(ground_truth, 'r', encoding='utf-8') as fg:
        out1 = [line.strip() for line in f1]
        out2 = [line.strip() for line in f2]
        gold = [line.strip() for line in fg]

    max_len = max(len(out1), len(out2), len(gold))
    out1 += [''] * (max_len - len(out1))
    out2 += [''] * (max_len - len(out2))
    gold += [''] * (max_len - len(gold))

    correct1 = correct2 = 0

    print(f"{'Idx':<5} | {'Truth':<20} | {'File 1 (top3)':<30} | {'File 2':<20} | Result")
    print("-" * 100)

    for i in range(max_len):
        t = gold[i]
        a = out1[i]
        b = out2[i]

        # File1 may have up to 3 suggestions separated by spaces
        suggestions1 = a.split()
        result = []
        if t in suggestions1:
            correct1 += 1
            result.append("OK1")
        if b == t:
            correct2 += 1
            result.append("OK2")

        print(f"{i+1:<5} | {t:<20} | {a or '[empty]':<50} | {b or '[empty]':<20} | {' '.join(result) or 'X'}")

    print("\nSummary")
    print("-" * 100)
    print(f"Total queries     : {max_len}")
    print(f"Correct (File 1)  : {correct1} ({correct1/max_len*100:.2f}%)")
    print(f"Correct (File 2)  : {correct2} ({correct2/max_len*100:.2f}%)")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python compare3.py suggestions.txt typos.txt true_answers.txt")
        sys.exit(1)

    compare3_files(sys.argv[1], sys.argv[2], sys.argv[3])