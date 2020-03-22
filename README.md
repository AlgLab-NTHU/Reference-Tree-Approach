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
   
     - RFT (parameter l is 6 and parameter k is 10)
       ```
       ./oRFT Dataset/text/Drosophila_Alphabet04_1M Dataset/pattern/Drosophila_patterns/len_0100/A04_1M_l0100_r1000_20_0001 6 10
       ```
     - RFT_acc (parameter l is 6 and parameter k is 10)
       ```
       ./oRFT_acc Dataset/text/Drosophila_Alphabet04_1M Dataset/pattern/Drosophila_patterns/len_0100/A04_1M_l0100_r1000_20_0001 6 10
       ```
     - sdsl_csa_S
       ```
       ./o_sdsl_csa_S Dataset/text/Drosophila_Alphabet04_1M Dataset/pattern/Drosophila_patterns/len_0100/A04_1M_l0100_r1000_20_0001
       ```
   - The Bible English example
   
     - RFT (parameter l is 9 and parameter k is 100)
       ```
       ./oRFT Dataset/text/Bible_Alphabet63_4M Dataset/pattern/Bible_patterns/len_0100/A63_4M_l0100_r1000_20_0001 9 100
       ```
     - RFT_acc (parameter l is 9 and parameter k is 100)
       ```
       ./oRFT_acc Dataset/text/Bible_Alphabet63_4M Dataset/pattern/Bible_patterns/len_0100/A63_4M_l0100_r1000_20_0001 9 100
       ```
     - sdsl_csa_S
       ```
       ./o_sdsl_csa_S Dataset/text/Bible_Alphabet63_4M Dataset/pattern/Bible_patterns/len_0100/A63_4M_l0100_r1000_20_0001
       ```
   The format of output message is "*x*(*y*)," where *x* is the position of the occurrence and *y* is the *y*th pattern.

# Author
  - Yi Kung Hsieh
