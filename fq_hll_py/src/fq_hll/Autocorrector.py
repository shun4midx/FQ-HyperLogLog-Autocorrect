############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: Autocorrector.py                   #
############################################

import sys, os
from typing import Dict, List, Union
root = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, 'src'))
sys.path.insert(0, root)

import math
import time
from dataclasses import dataclass
from .HyperLogLog import SketchConfig, HyperLogLog
from bitarray import bitarray

def extract_qgrams(word, q=2, fuzzier=False):
    if len(word) < q:
        return []
    if q != 2:
        return [word[i : i + q] for i in range(len(word) - q + 1)]
    else:
        qgrams = []
        
        for i in range(len(word) - q + 1): # Include empty combinations
            qgram = word[i : i + q]
            qgrams.append(qgram)
            qgrams.append(qgram)
            qgrams.append(f"{qgram[0]} ") # Space is seen as empty
            qgrams.append(f" {qgram[1]}")

            if fuzzier:
                qgrams.append(f"{qgram[1]}{qgram[0]}")

        return qgrams

# ======== REPACKAGING ======== #
def is_valid(word, letters=None):
    if letters is None: # Empty can mean all letters
        return True
    else:
        return all(ch in letters for ch in word.lower())

def load_words(src, letters=None):
    """
    src: either
      * a list/tuple of words
      * a filename (one word per line)
      * a string
    return_display: if True, also return {lowercase: original} map
    """
    # 1) Already a Python sequence?
    if isinstance(src, (list, tuple)):
        raw = src

    # 2) File on disk?
    elif isinstance(src, str) and os.path.isfile(src):
        with open(src, 'r', encoding='utf-8') as f:
            raw = [line.strip() for line in f if line.strip()]

    # 3) String?
    elif isinstance(src, str):
        raw = [src]

    else:
        raise ValueError(f"`src` ({src}) must be a list/tuple, a path, or a string")

    # Now raw is a list of original words
    words = [word.lower() for word in raw if is_valid(word, letters)]
    display = {word.lower(): word for word in raw if is_valid(word, letters)}
    
    return words, display

addon_files = ["texting"] # Files to addon 20k_shun4midx.txt

@dataclass
class Results:
    suggestions: Dict[str, Union[str, List[str]]]
    scores: Dict[str, Union[float, List[float]]]

class Autocorrector:
    # Default non-custom imported keyboards here would disregarded special characters (such as commas, not things like é and ö). Please import your own if you need to. I consider number rows too.
    # Of course, Dvorak is not as intuitive. I replaced special characters with a whitespace for sake of consistency.
    # "a-z" only considers English letters. For French, for example, you can import valid_letters = ["a-z", "é", "É", "à", "À", "ê". "Ê". "è", "È"]
    def __init__(self, dictionary_list=os.path.join("test_files", "20k_shun4midx.txt"), valid_letters = "a-z", keyboard = "qwerty", *, alpha=0.2, beta=0.35, b=10):
        # Deal with allowed letters only
        if valid_letters in (None, "", []):
            self.letters = None
        elif isinstance(valid_letters, str):
            valid_letters = [valid_letters] if valid_letters != "" else []
        elif not isinstance(valid_letters, list):
            raise ValueError(f"`{valid_letters}` is not a string or a list of strings")
        
        letters = []

        for letter in valid_letters:
            if (letter == "a-z"):
                letters += [chr(ord('a') + i) for i in range(26)]
            elif (letter == "0-9"):
                letters += [chr(ord('0') + i) for i in range(10)]
            elif len(letter) == 1 and letter != " ":
                letters.append(letter.lower() if letter.lower() else letter)
            else:
                raise ValueError(f"`{valid_letters}` should contain single character non-space letters only, other than abbreviations a-z and 0-9")

        self.letters = set(letters) if letters != [] else None

        # Deal with keyboard
        self.keyboard = []

        if keyboard == "qwerty":
            self.keyboard = ["1234567890", "qwertyuiop", "asdfghjkl", "zxcvbnm"]
        elif keyboard == "azerty":
            self.keyboard = ["1234567890", "azertyuiop", "qsdfghjklm", "wxcvbn"]
        elif keyboard == "qwertz":
            self.keyboard = ["1234567890", "qwertzuiopü", "asdfghjklöä", "yxcvbnm"]
        elif keyboard == "dvorak":
            self.keyboard = ["1234567890", "'  pyfgcrl", "aoeuidhtns", " qjkxbmwvz"]
        elif keyboard == "colemak":
            self.keyboard = ["1234567890", "qwfpgjluy", "arstdhneio", "zxcvbkm"]
        else:
            if not isinstance(keyboard, list):
                raise ValueError('''`keyboard` should be a list containing the letters of each row in a string, from top to bottom, or one of the following abbreviations: "qwerty', "azerty", "qwertz", "dvorak", "colemak".''')
            self.keyboard = keyboard

        self.KEY_POS = {}

        for i in range(len(self.keyboard)):
            for j in range(len(self.keyboard[i])):
                self.KEY_POS[self.keyboard[i][j]] = (i, j)

        # Deal with dictionary
        if isinstance(dictionary_list, list):
            # If a list is provided, use it directly
            self.word_dict, self.display_map = load_words(dictionary_list, self.letters)
        elif dictionary_list in addon_files:
            # Read both 20k_shun4midx and the addon_file
            base_dir = os.path.dirname(os.path.abspath(__file__))
            base_path = os.path.join(base_dir, "test_files", "20k_shun4midx.txt")
            addon_path = os.path.join(base_dir, "test_files", dictionary_list + ".txt")

            new_dict = []

            with open(base_path, 'r', encoding='utf-8') as f:
                for line in f:
                    if line.strip():
                        new_dict.append(line.strip())

            with open(addon_path, 'r', encoding='utf-8') as f:
                for line in f:
                    if line.strip():
                        new_dict.append(line.strip())

            self.word_dict, self.display_map = load_words(new_dict, self.letters)
        elif os.path.isfile(dictionary_list):
            # If a file path is provided, load the dictionary from the file
            dictionary_path = os.path.abspath(dictionary_list)
            self.word_dict, self.display_map = load_words(dictionary_path, self.letters)
        else:
            # Fall back to the default file relative to this script
            base_dir = os.path.dirname(os.path.abspath(__file__))
            dictionary_path = os.path.join(base_dir, dictionary_list)
            if not os.path.isfile(dictionary_path):
                raise FileNotFoundError(f"Default dictionary file not found: {dictionary_path}")
            self.word_dict, self.display_map = load_words(dictionary_path, self.letters)
        
        self.alpha, self.beta, self.b = alpha, beta, b

        self.save_dictionary() # Don't have to recompute every time

        # Compute removed
        self.removed_words = set()
        self.compact_threshold = 0.1

    def key_dist(self, a, b):
        xa, ya = self.KEY_POS.get(a, (0,0))
        xb, yb = self.KEY_POS.get(b, (0,0))
        dx, dy = xa - xb, ya - yb
        return math.sqrt(dx*dx + dy*dy)
    
    def word_dist(self, a, b):
        na, nb = len(a), len(b)

        prev = [float(j) for j in range(nb+1)]
        curr = [0.0] * (nb + 1)

        for i in range(1, na + 1):
            curr[0] = float(i)
            ai = a[i - 1].lower()
            for j in range(1, nb + 1):
                bj = b[j - 1].lower()
                # substitution = prev[j - 1] + key_distance(a[i - 1], b[j - 1])
                cost_sub = prev[j - 1] + self.key_dist(ai, bj)
                # deletion = prev[j] + 1, insertion = curr[j - 1] + 1
                cost_del = prev[j] + 1.0
                cost_ins = curr[j - 1] + 1.0
                curr[j] = min(cost_sub, cost_del, cost_ins)
            prev, curr = curr, prev

        return prev[nb]

    def is_valid(self, word):
        return is_valid(word, self.letters)

    def save_dictionary(self):
        self.start_total = time.perf_counter()
        self.t0 = time.perf_counter()

        self.q = 2 
        self.cfg = SketchConfig(b=self.b)
        self.qgram_sketches = {}

        # Precompute all dictionary q‑grams + FQ‑HLL sketches
        self.WORD_COUNT = len(self.word_dict)
        self.NUM_BUCKETS = 2 ** math.ceil(math.log2(self.WORD_COUNT) / 2)
        self.BUCKET_SIZE = math.ceil(self.WORD_COUNT / self.NUM_BUCKETS)

        # 1) Build FQ‑HLL per q‑gram
        for idx, word in enumerate(self.word_dict):
            qgrams = extract_qgrams(word, self.q, fuzzier=False)
            
            # For fuzzy‑HLL: shift by Zipf bucket, more shift = more common
            bucket_idx = min(self.NUM_BUCKETS, ((idx + 1) // self.BUCKET_SIZE) + 1)
            shift = min(int(math.floor(math.log2(self.NUM_BUCKETS / bucket_idx))) * 4, 64)
            
            for gram in qgrams:
                if gram not in self.qgram_sketches:
                    self.qgram_sketches[gram] = HyperLogLog(self.cfg)
                self.qgram_sketches[gram].shifted_insert(f"{gram}_{word}", shift)

        # 2) Precompute dict‐word q‑gram sets for Jaccard
        self.t1 = time.perf_counter()

        self.all_qgrams = sorted(self.qgram_sketches.keys())
        self.qgram_idx = {gram : idx for idx, gram in enumerate(self.all_qgrams)}
        self.TOTAL_QGRAMS = len(self.all_qgrams)

        self.word_bits = []
        for word in self.word_dict:
            ba = bitarray(self.TOTAL_QGRAMS)
            ba.setall(0)
            for gram in extract_qgrams(word, q=self.q, fuzzier=False):
                ba[self.qgram_idx[gram]] = 1
            self.word_bits.append(ba)

    def add_dictionary(self, to_be_added):
        words, displays = load_words(to_be_added, self.letters)
        added = []

        for word in words:
            if word not in self.word_dict:
                added.append(word)
            else:
                self.removed_words.discard(word)

        if not added:
            return added

        old_exp = int(math.log2(self.NUM_BUCKETS))
        new_exp = math.ceil(math.log2(len(self.word_dict) + len(added)) / 2)

        # 1) Full rebuild if exponent bumps
        if new_exp != old_exp:
            self.word_dict.extend(added)
            for word in added:
                self.display_map[word] = displays[word]
            self.save_dictionary()
            return added

        # 2) Otherwise true O(1) per‑word work:
        # Recompute NUM_BUCKETS (BUCKET_SIZE stays frozen)
        self.NUM_BUCKETS = math.ceil((len(self.word_dict) + len(added)) / self.BUCKET_SIZE)

        # For each new word: update display_map, HLL sketches, and one bitarray
        base = len(self.word_dict)
        for idx, word in enumerate(added):
            self.word_dict.append(word)
            self.display_map[word] = displays[word]

            # Qgram sketches
            qgrams = extract_qgrams(word, self.q, fuzzier=False)
            bucket_idx = ((base + idx + 1) // self.BUCKET_SIZE) + 1
            shift = min(int(math.floor(math.log2(self.NUM_BUCKETS / bucket_idx))) * 4, 64)
            for gram in qgrams:
                sketch = self.qgram_sketches.setdefault(gram, HyperLogLog(self.cfg))
                sketch.shifted_insert(f"{gram}_{word}", shift)

        # If new qgrams appeared, extend existing bitarrays by zeros
        new_all = sorted(self.qgram_sketches)
        delta = len(new_all) - len(self.all_qgrams)
        if delta > 0:
            for ba in self.word_bits:
                ba.extend([0] * delta)
            self.all_qgrams = new_all
            self.qgram_idx = {gram: idx for idx, gram in enumerate(new_all)}
            self.TOTAL_QGRAMS = len(new_all)

        # Finally append one bitarray per new word
        for w in added:
            ba = bitarray(self.TOTAL_QGRAMS); ba.setall(0)
            for gram in extract_qgrams(w, self.q, fuzzier=False):
                ba[self.qgram_idx[gram]] = 1
            self.word_bits.append(ba)

        return added

    def remove_dictionary(self, to_be_removed):
        words, _ = load_words(to_be_removed, self.letters)

        removed = []

        for word in words:
            if word in self.word_dict:
                self.removed_words.add(word)
                removed.append(word)
        
        # If too many removed, full rebuild:
        if len(self.removed_words) >= len(self.word_dict) * self.compact_threshold:
            self.word_dict = [word for word in self.word_dict if word not in self.removed_words]
            self.display_map = {word: self.display_map[word] for word in self.word_dict}
            self.removed_words.clear()
            self.save_dictionary()
        
        return removed

    def autocorrect(self, queries_list, output_file="None", use_keyboard=True, return_invalid_words=True, print_details=False, print_times=False):
        if print_times:
            self.save_dictionary()

        queries, query_displays = load_words(queries_list)

        # 3) Process queries
        self.t2 = time.perf_counter()

        output = []
        suggestions = {} # Dictionary
        final_scores = {}

        for query in queries:
            if not self.is_valid(query):
                if return_invalid_words:
                    suggestions[query_displays[query]] = query_displays[query]
                    final_scores[query_displays[query]] = 0.0
                    output.append(query_displays[query])
                    continue
                else:
                    suggestions[query_displays[query]] = ""
                    final_scores[query_displays[query]] = 0.0
                    output.append("")
                    continue

            # Print fuzzy HLL estimates per gram
            Q = set(extract_qgrams(query, self.q, fuzzier=True)) # Qgrams
            if print_details:
                print(f"{query:>12} -> qgrams: ", end="|")
                for gram in set(Q):
                    est = self.qgram_sketches[gram].estimate() if gram in self.qgram_sketches else 0
                    print(f"{gram}({est}) ", end="|")
                        
                print()

            # Build query bitarray
            qb = bitarray(self.TOTAL_QGRAMS)
            qb.setall(0)

            for gram in Q:
                if gram in self.qgram_idx:
                    qb[self.qgram_idx[gram]] = 1
            qb_count = qb.count()

            # Scan all words for candidates (intersecting grams >= 1)
            cand_idxs = []
            for idx, wb in enumerate(self.word_bits):
                if self.word_dict[idx] in self.removed_words:
                    continue
                inter = (wb & qb).count()
                if inter > 0:
                    cand_idxs.append((idx, inter))

            if not cand_idxs:
                if return_invalid_words:
                    if print_details:
                        print(f"  -> no overlaps; returning original: {query_displays[query]}")
                    suggestions[query_displays[query]] = query_displays[query]
                    final_scores[query_displays[query]] = 0.0
                    output.append(query_displays[query])
                    continue
                else:
                    if print_details:
                        print("  -> no overlaps; returning empty")
                    suggestions[query_displays[query]] = ""
                    final_scores[query_displays[query]] = 0.0
                    output.append("")
                    continue

            # d) Compute Jaccard & Zipf score R for each candidate
            J = {} # Jaccards
            R = {} # Ranks
            for idx, inter in cand_idxs:
                uni = qb_count + self.word_bits[idx].count() - inter
                J[idx] = inter / uni if uni else 0.0
                R[idx] = 1 / (((idx + 1) // self.BUCKET_SIZE) + 1) # same Zipf normalization as before

            # e) Sweep tau to pick best
            best_idx = None
            best_score = -1.0
            best_tau = None

            for tau in (0.8, 0.7, 0.6, 0.5, 0.4):
                passers = [idx for idx in J if J[idx] >= tau]
                if not passers:
                    continue
                for idx in passers:
                    length_penalty = 1 - ((abs(len(self.word_dict[idx]) - len(query)) / len(query)) ** 2)
                    norm_dist = 1 / (1 + self.word_dist(query, self.word_dict[idx])) if use_keyboard else 1
                    score = (J[idx] + self.alpha * R[idx]) * length_penalty + norm_dist * self.beta + (1 if query == self.word_dict[idx] else 0)
                    if score > best_score:
                        best_score = score
                        best_idx = idx
                        best_tau = tau

            if best_idx is None: # Fallback, take highest
                best_idx = max(J, key=J.get)
                best_score = J[best_idx]
                best_tau = 0.4

            picked = self.word_dict[best_idx]
            if print_details:
                print(f"{query:>12} -> picked {picked!r} at tau={best_tau}"
                    f"  (J={J[best_idx]}, score={best_score})")
                print("-" * 30)

            displayed_picked = self.display_map.get(picked, picked)
            suggestions[query_displays[query]] = displayed_picked
            final_scores[query_displays[query]] = best_score
            output.append(displayed_picked)

        # 4) Write out
        self.t3 = time.perf_counter()

        if output_file != "None":
            with open(output_file, "w") as out:
                out.write("\n".join(output))

        # 5) Output time elapsed
        self.end_total = time.perf_counter()
        if print_times:
            print(f"Build sketches:    {self.t1 - self.t0:.3f}s")
            print(f"Build bit‑vectors: {self.t2 - self.t1:.3f}s")
            print(f"Query processing:  {self.t3 - self.t2:.3f}s")
            print(f"Total autocorrect: {self.end_total - self.start_total:.3f}s")

        # Return
        return Results(suggestions=suggestions, scores=final_scores)

    def top3(self, queries_list, output_file="None", use_keyboard=True, return_invalid_words=True, print_details=False, print_times=False):
        if print_times:
            self.save_dictionary()

        queries, query_displays = load_words(queries_list)

        # 3) Process queries
        self.t2 = time.perf_counter()

        output = [] # Output
        suggestions = {} # Dictionary
        final_scores = {}

        for query in queries:
            if not self.is_valid(query):
                if return_invalid_words:
                    suggestions[query_displays[query]] = [query_displays[query], "", ""]
                    final_scores[query_displays[query]] = [0.0, 0.0, 0.0]
                    output.append(f"{query_displays[query]}  ")
                    continue
                else:
                    suggestions[query_displays[query]] = ["", "", ""]
                    final_scores[query_displays[query]] = [0.0, 0.0, 0.0]
                    output.append("")
                    continue

            # Print fuzzy HLL estimates per gram
            Q = set(extract_qgrams(query, self.q, fuzzier=True)) # Qgrams
            if print_details:
                print(f"{query:>12} -> qgrams: ", end="|")
                for gram in set(Q):
                    est = self.qgram_sketches[gram].estimate() if gram in self.qgram_sketches else 0
                    print(f"{gram}({est}) ", end="|")
                        
                print()

            # Build query bitarray
            qb = bitarray(self.TOTAL_QGRAMS)
            qb.setall(0)

            for gram in Q:
                if gram in self.qgram_idx:
                    qb[self.qgram_idx[gram]] = 1
            qb_count = qb.count()

            # Scan all words for candidates (intersecting grams >= 1)
            cand_idxs = []
            for idx, wb in enumerate(self.word_bits):
                if self.word_dict[idx] in self.removed_words:
                    continue
                inter = (wb & qb).count()
                if inter > 0:
                    cand_idxs.append((idx, inter))

            if not cand_idxs:
                if return_invalid_words:
                    if print_details:
                        print(f"  -> no overlaps; returning original: {query_displays[query]}  ")
                    suggestions[query_displays[query]] = [query_displays[query], "", ""]
                    final_scores[query_displays[query]] = [0.0, 0.0, 0.0]
                    output.append(f"{query_displays[query]}  ")
                    continue
                else:
                    if print_details:
                        print("  -> no overlaps; returning empty")
                    suggestions[query_displays[query]] = ["", "", ""]
                    final_scores[query_displays[query]] = [0.0, 0.0, 0.0]
                    output.append("")
                    continue

            # d) Compute Jaccard & Zipf score R for each candidate
            J = {} # Jaccards
            R = {} # Ranks
            for idx, inter in cand_idxs:
                uni = qb_count + self.word_bits[idx].count() - inter
                J[idx] = inter / uni if uni else 0.0
                R[idx] = 1 / (((idx + 1) // self.BUCKET_SIZE) + 1) # same Zipf normalization as before

            # e) Sweep tau to pick top 3
            # Build Jaccard + Zipf score for all candidates
            scored = []
            for idx in J:
                length_penalty = 1 - ((abs(len(self.word_dict[idx]) - len(query)) / len(query)) ** 2)
                base_score = (J[idx] + self.alpha * R[idx]) * length_penalty
                scored.append((base_score, idx))

            # Take only the top‑`shortlist` by base_score
            scored.sort(key=lambda x: x[0], reverse=True)
            shortlist = scored[:30]

            # If use_keyboard, recompute with keyboard distance
            final = []
            for base_score, idx in shortlist:
                if use_keyboard:
                    norm_dist = 1 / (1 + self.word_dist(query, self.word_dict[idx]))
                    score = base_score + self.beta * norm_dist + (1 if query==self.word_dict[idx] else 0)
                else:
                    score = base_score + (1 if query==self.word_dict[idx] else 0)
                final.append((score, idx))

            # Sort and return top 3
            final.sort(key=lambda x: x[0], reverse=True)
            seen = set()
            top3 = []
            top3_scores = []
            for score, idx in final:
                suggestion = self.display_map[self.word_dict[idx]]
                if suggestion not in seen:
                    seen.add(suggestion)
                    top3.append(suggestion)
                    top3_scores.append(score) 
                    if len(top3) == 3:
                        break

            while len(top3) < 3:
                top3.append("")
                top3_scores.append(0.0)
        
            if print_details:
                print(f"{query:>12} -> top 3: {top3}")
                print("-" * 30)

            suggestions[query_displays[query]] = top3
            final_scores[query_displays[query]] = top3_scores
            output.append(" ".join(top3))

        # 4) Write out
        self.t3 = time.perf_counter()
        if output_file != "None":
            with open(output_file, "w") as out:
                out.write("\n".join(output))

        # 5) Output time elapsed
        self.end_total = time.perf_counter()
        if print_times:
            print(f"Build sketches:    {self.t1 - self.t0:.3f}s")
            print(f"Build bit‑vectors: {self.t2 - self.t1:.3f}s")
            print(f"Query processing:  {self.t3 - self.t2:.3f}s")
            print(f"Total autocorrect: {self.end_total - self.start_total:.3f}s")

        # Return
        return Results(suggestions=suggestions, scores=final_scores)

# ======== SAMPLE USAGE ======== #
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