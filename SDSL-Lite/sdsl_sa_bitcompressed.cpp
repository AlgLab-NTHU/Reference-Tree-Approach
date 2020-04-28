#include <sdsl/suffix_arrays.hpp>
#include <iostream>
#include <chrono>
#include <fstream>

using namespace sdsl;
using namespace std;
using namespace std::chrono;

int main (int argc, char **argv)
{
    int numberPattern = 0;
    int *lengthPattern = NULL;
    string *pattern = NULL;
    int i = 0;
    high_resolution_clock::time_point preprocessing_start;
    high_resolution_clock::time_point preprocessing_end;
    high_resolution_clock::time_point searching_start;
    high_resolution_clock::time_point searching_end;

    // check the number of input arguments
    if (argc != 3)
    {
        // if number is incorrect, stop
        cout << "Usage: " << argv[0] << " [text file name] [pattern file name]" << endl;
        return 0;
    }

    // open the file of patterns
    ifstream inputPatternFile(argv[2], ios::in | ios::binary);
    if (!inputPatternFile)
    {
        // if open is failed, stop
        cout << "Cannot read the pattern file: " << argv[2] << endl;
        return 0;
    }

    // obtain the number of patterns in the first line
    inputPatternFile >> numberPattern;
    lengthPattern = new int[numberPattern];
    pattern = new string[numberPattern];
    // obtain the patterns in the following lines
    for (i = 0; i < numberPattern; i++)
    {
        // obtain the length of this pattern
        inputPatternFile >> lengthPattern[i];
        inputPatternFile.get();
        pattern[i].resize(lengthPattern[i]);
        // obtain this pattern
        inputPatternFile.read(&pattern[i][0], lengthPattern[i]);
        inputPatternFile.get();
    }
    // close the file
    inputPatternFile.close();

    // set the time stamp for the start of preprocessing
    preprocessing_start = high_resolution_clock::now();

    // use SA algorithm in SDSL-lite
    csa_bitcompressed<> csa;
    ifstream in(argv[1]);

    if (!in)
    {
        cout << "ERROR: cannot read the text file (\"" << argv[1] << "\")" << endl;
        return 0;
    }

    // create the SA
    construct(csa, argv[1], 1);

    size_t j = 0;
    auto occs = 0;

    // set the time stamp for the end of preprocessing
    preprocessing_end = high_resolution_clock::now();
    // set the time stamp for the start of searching
    searching_start = high_resolution_clock::now();

    for (i = 0; i < numberPattern; i++)
    {
        // the result is stored in occs
        auto occs = locate(csa, pattern[i].begin(), pattern[i].end());
        // sort the result by positions (in order to fit the output format of reference tree approach)
        // this sort would not influence the searching time (under the normal case that occurrences for this pattern are not too many)
        // sort(occs.begin(), occs.end());
        // output the result
        for (j = 0; j < occs.size(); j++)
            cout << occs[j] + lengthPattern[i] << "(" << i + 1 << "),";
    }
    // set the time stamp for the end of searching
    searching_end = high_resolution_clock::now();

    // compute the time for preprocessing and searching
    duration<double> preprocessing_time_span = duration_cast<duration<double>>(preprocessing_end - preprocessing_start);
    duration<double> searching_time_span = duration_cast<duration<double>>(searching_end - searching_start);

    // output the time of preprocessing and searching
    cout.precision(6);
    cout << "\t" << fixed << preprocessing_time_span.count() << "\t" << fixed << searching_time_span.count() << "\t" << fixed << preprocessing_time_span.count() + searching_time_span.count() << endl;

    // release the memory for CSA
    delete [] lengthPattern;
    delete [] pattern;

    return 0;
}
