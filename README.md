# HyperLogLog-Autocorrect
An improved HyperLogLog (HLL) Autocorrection Library based on my final project in **Advanced Data Structures 2025**, with credit to my team members at the time. For their respect of privacy, I would keep them anonymous, unless they request otherwise.

## Context
For context, for our final project, we focused on Autocorrection. We implemented one [baseline algorithm (LDP with Bloom and Cuckoo filters)](https://arxiv.org/pdf/2208.05264), and I was also tasked to design an Autocorrection algorithm using HLL. HLL demonstrated decent performance (accuracy, speed, and memory) compared to the baseline with preliminary trials, but in our evaluation, we realized the way we used word dictionaries to run it didn't give HLL a fair chance.

After the semester, I doubt my team members have much interest still, but I still have some interest in furthering it and making it better and seeing how HLL would actually perform if given a fair chance.

## Notes
 - HLL naturally doesn't have Local Differential Privacy (LDP) yet, but has natural obfuscation.
 - This library does not collect personal data. However, given the above point, use it at your own discretion.

## Plans for the Repo
 - Develop a Python library importable via `pip install`
 - [Maybe] Include a C++ library too that is importable via CMake, since as most of you know, I love C++.
 - Formally document the logic behind the algorithm via a LaTeX file (or its PDF directly).
 - If time permits, I may include a formal proof of the HLL algorithm's validity and potential developments in LDP.
