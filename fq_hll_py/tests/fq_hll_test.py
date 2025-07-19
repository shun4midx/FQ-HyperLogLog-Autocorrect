############################################
# Copyright (c) 2025 Shun/翔海 (@shun4midx) #
# Project: FQ-HyperLogLog-Autocorrect      #
# File Type: Python file                   #
# File: fq_hll_test.py                     #
############################################

from fq_hll import Autocorrector, compare_files, compare3_files

if __name__ == "__main__":
    # ======== 20k_shun4midx.txt ======== #
    ac = Autocorrector()

    # Test files
    autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/20k_autocorrect_suggestions.txt", True, True) # Print details and print times
    autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/20k_autocorrect_suggestions.txt", False, True) # Don't print details but print times
    # print(autocor)

    top3_ans = ac.top3("test_files/typo_file.txt", "outputs/20k_top3_suggestions.txt", True, True) # Print details and print times
    top3_ans = ac.top3("test_files/typo_file.txt", "outputs/20k_top3_suggestions.txt", False, True) # Don't print details but print times
    # print(top3_ans)

    # Run files
    compare_files("outputs/20k_autocorrect_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt")
    compare3_files("outputs/20k_top3_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt")

    # ======== database.txt ======== #
    ac = Autocorrector("test_files/database.txt") # It is able to search within the src folder first, before searching in the user's folder

    # Test files
    autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/database_autocorrect_suggestions.txt", True, True) # Print details and print times
    autocor = ac.autocorrect("test_files/typo_file.txt", "outputs/database_autocorrect_suggestions.txt", False, True) # Don't print details but print times
    # print(autocor)

    top3_ans = ac.top3("test_files/typo_file.txt", "outputs/database_top3_suggestions.txt", True, True) # Print details and print times
    top3_ans = ac.top3("test_files/typo_file.txt", "outputs/database_top3_suggestions.txt", False, True) # Don't print details but print times
    # print(top3_ans)

    # Run files
    compare_files("outputs/database_autocorrect_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt")
    compare3_files("outputs/database_top3_suggestions.txt", "test_files/typo_file.txt", "test_files/output_compare.txt")