# Reference-Tree-Approach
Reference Tree Approach is for solving the Exact Multiple Pattern Matching Problem

# Introduction
Reference Tree Approach is developed in C Language. It contains two phases, preprocessing and searching phases. In preprocessing phase, all the substrings with length l (the user pre-specified parameter) of text string will be constructed into a reference tree. In searching phase, the search can find the possible occurrences of pattern(s) in text string via the guiding of the reference tree and then verify possible occurrences to obtain the result.

# Requirements
- The GNU Compiler Collection - 4.8.5 and above

*The programs in SDSL-Lite directory need SDSL-lite (SDSL 2.0) library.
(Please consider [SDSL 2.0](https://github.com/simongog/sdsl-lite) to install the library.)

# Getting Started
1. Reference Tree Approach
    To compile the program in Reference Tree (RFT.c/RFT_acc.c) using gcc run:
    ```
    gcc -Wall -O3 -o program program.c
    ```
    i.e.:
    ```
    gcc -Wall -O3 -o oRFT RFT.c
    gcc -Wall -O3 -o oRFT_acc RFT_acc.c
    ```

2. The algorithms in SDSL-Lite (CSA-S, CSA-GGV, SA, CST-OFG, CST-RNO and CST-S)
   ```
   g++ -std=c++11 -O3 -DNDEBUG -I ~/include -L ~/lib program.cpp -o program -lsdsl -ldivsufsort -ldivsufsort64
   ```
   i.e.:
   ```
   g++ -std=c++11 -O3 -DNDEBUG -I ~/include -L ~/lib sdsl_csa_S.cpp -o o_sdsl_csa_S -lsdsl -ldivsufsort -ldivsufsort64
   ```

3. Run the examples in Dataset
   - The Drosophila DNA sequence example
     RFT:
     ```
     ./oRFT Dataset/text/Drosophila_Alphabet04_1M Dataset/pattern/Drosophila_patterns/len_0100/A04_1M_l0100_r1000_20_0001 6 10
     ```
     RFT_acc
     ```
     ./oRFT_acc Dataset/text/Drosophila_Alphabet04_1M Dataset/pattern/Drosophila_patterns/len_0100/A04_1M_l0100_r1000_20_0001 6 10
     ```
     where 6 is parameter l and 10 is parameter k (user pre-specified parameters)
   - The Bible English example
     ```
     ./oRFT Dataset/text/Bible_Alphabet63_4M Dataset/pattern/Bible_patterns/len_0100/A63_4M_l0100_r1000_20_0001 9 100
     ```
     where 9 is parameter l and 100 is parameter k (user pre-specified parameters)
