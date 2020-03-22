# Reference-Tree-Approach
Reference Tree Approach is for solving the Exact Multiple Pattern Matching Problem

# Introduction
Reference Tree Approach is developed in C Language. It contains two phases, preprocessing and searching phases. In preprocessing phase, all the substrings with length l (the user pre-specified parameter) of text string will be constructed into a reference tree. In searching phase, the search can find the possible occurrences of pattern(s) in text string via the guiding of the reference tree and then verify possible occurrences to obtain the result.

# Requirements
The GNU Compiler Collection - 4.8.5 and above

*The programs in SDSL-lite directory need SDSL-lite (SDSL 2.0) library.
(Please consider https://github.com/simongog/sdsl-lite to install the library.)

#Getting Started
To compile the program in Reference Tree (RFT.c/RFT_acc.c), using gcc run:
gcc -Wall -O3 -o program program.c
i.e.:
gcc -Wall -O3 -o oRFT RFT.c
gcc -Wall -O3 -o oRFT_acc RFT_acc.c
