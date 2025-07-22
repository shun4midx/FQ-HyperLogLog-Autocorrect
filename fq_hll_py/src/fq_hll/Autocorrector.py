import sys, os, ast
root = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, 'src'))
sys.path.insert(0, root)

import math
import time
from .HyperLogLog import SketchConfig, HyperLogLog
from bitarray import bitarray

def extract_qgrams(word, q=2, swap=False):
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

            if swap:
                qgrams.append(f"{qgram[1]}{qgram[0]}")

        return qgrams

# ======== REPACKAGING ======== #
def load_words(src):
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
        return [src]

    else:
        raise ValueError(f"`src` ({src}) must be a list/tuple, a path, or a string")

    # now raw is a list of original words
    words = [word.lower() for word in raw]

    display = {word.lower(): word for word in raw}
    return words, display

addon_files = ["texting"] # Files to addon 20k_shun4midx.txt

class Autocorrector:
    def __init__(self, dictionary_list=os.path.join("test_files", "20k_shun4midx.txt"), *, alpha=0.3, b=10):
        if isinstance(dictionary_list, list):
            # If a list is provided, use it directly
            self.word_dict = dictionary_list
            self.display_map = {word: word for word in dictionary_list}
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

            self.word_dict, self.display_map = load_words(new_dict)
        elif os.path.isfile(dictionary_list):
            # If a file path is provided, load the dictionary from the file
            dictionary_path = os.path.abspath(dictionary_list)
            self.word_dict, self.display_map = load_words(dictionary_path)
        else:
            # Fall back to the default file relative to this script
            base_dir = os.path.dirname(os.path.abspath(__file__))
            dictionary_path = os.path.join(base_dir, dictionary_list)
            if not os.path.isfile(dictionary_path):
                raise FileNotFoundError(f"Default dictionary file not found: {dictionary_path}")
            self.word_dict, self.display_map = load_words(dictionary_path)
        
        self.alpha, self.b = alpha, b

        self.save_dictionary() # Don't have to recompute every time

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
            qgrams = extract_qgrams(word, self.q, swap=False)
            
            # For fuzzy‑HLL: shift by Zipf bucket, more shift = more common
            bucket_idx = min(self.NUM_BUCKETS, (idx + 1 // self.BUCKET_SIZE) + 1)
            shift = min(int(math.floor(math.log2(self.NUM_BUCKETS / bucket_idx))) * 4, 64)
            
            for gram in qgrams:
                if gram not in self.qgram_sketches:
                    self.qgram_sketches[gram] = HyperLogLog(self.cfg)
                self.qgram_sketches[gram].shifted_insert(f"{gram}_u{idx + 1}", shift)

        # 2) Precompute dict‐word q‑gram sets for Jaccard
        self.t1 = time.perf_counter()

        self.all_qgrams = sorted(self.qgram_sketches.keys())
        self.qgram_idx = {gram : idx for idx, gram in enumerate(self.all_qgrams)}
        self.TOTAL_QGRAMS = len(self.all_qgrams)

        self.word_bits = []
        for word in self.word_dict:
            ba = bitarray(self.TOTAL_QGRAMS)
            ba.setall(0)
            for gram in extract_qgrams(word, q=self.q, swap=False):
                ba[self.qgram_idx[gram]] = 1
            self.word_bits.append(ba)

    def autocorrect(self, queries_list, output_file="None", print_details=False, print_times=False):
        if print_times:
            self.save_dictionary()

        queries, query_displays = load_words(queries_list)

        # 3) Process queries
        self.t2 = time.perf_counter()

        output = []
        suggestions = {} # Dictionary

        for query in queries:
            # Print fuzzy HLL estimates per gram
            Q = set(extract_qgrams(query, self.q, swap=True)) # Qgrams
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
                inter = (wb & qb).count()
                if inter > 0:
                    cand_idxs.append((idx, inter))

            if not cand_idxs:
                if print_details:
                    print("  -> no overlaps; returning empty")
                suggestions[query_displays[query]] = ""
                output.append("")
                continue

            # d) Compute Jaccard & Zipf score R for each candidate
            J = {} # Jaccards
            R = {} # Ranks
            for idx, inter in cand_idxs:
                uni = qb_count + self.word_bits[idx].count() - inter
                J[idx] = inter / uni if uni else 0.0
                R[idx] = 1 / ((idx + 1 // self.BUCKET_SIZE) + 1) # same Zipf normalization as before

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
                    score = (J[idx] + self.alpha * R[idx]) * length_penalty
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
        return suggestions

    def top3(self, queries_list, output_file="None", print_details=False, print_times=False):
        if print_times:
            self.save_dictionary(True)

        queries, query_display = load_words(queries_list)

        # 3) Process queries
        self.t2 = time.perf_counter()

        output = [] # Output
        suggestions = {} # Dictionary

        for query in queries:
            # Print fuzzy HLL estimates per gram
            Q = set(extract_qgrams(query, self.q, swap=True)) # Qgrams
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
                inter = (wb & qb).count()
                if inter > 0:
                    cand_idxs.append((idx, inter))

            if not cand_idxs:
                if print_details:
                    print("  -> no overlaps; returning empty")
                output.append("")
                suggestions[query_display[query]] = ["", "", ""]
                continue

            # d) Compute Jaccard & Zipf score R for each candidate
            J = {} # Jaccards
            R = {} # Ranks
            for idx, inter in cand_idxs:
                uni = qb_count + self.word_bits[idx].count() - inter
                J[idx] = inter / uni if uni else 0.0
                R[idx] = 1 / ((idx + 1 // self.BUCKET_SIZE) + 1) # same Zipf normalization as before

            # e) Sweep tau to pick top 3
            scored = []
            for idx in J:
                length_penalty = 1 - ((abs(len(self.word_dict[idx]) - len(query)) / len(query)) ** 2)
                score = (J[idx] + self.alpha * R[idx]) * length_penalty
                scored.append((score, idx))

            # Sort descending and take up to 3
            scored.sort(key=lambda x: x[0], reverse=True)
            top3 = [self.display_map.get(self.word_dict[idx], self.word_dict[idx]) for _, idx in scored[:3]]

            if print_details:
                print(f"{query:>12} -> top 3: {top3}")
                print("-" * 30)

            suggestions[query_display[query]] = top3
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
        return suggestions

# ======== SAMPLE USAGE ======== #
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

    # # You can even have a custom dictionary!
    dictionary = ["apple", "banana", "grape", "orange"]
    custom_ac = Autocorrector(dictionary)

    ans7 = custom_ac.autocorrect(["applle", "banana", "banan", "orenge", "grap", "pineapple"])
    ans8 = custom_ac.top3(["applle", "banana", "banan", "orenge", "grap", "pineapple"])

    print(ans7)
    print(ans8)