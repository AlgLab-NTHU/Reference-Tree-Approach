#include <sdsl/suffix_trees.hpp>
#include <iostream>
#include <chrono>
#include <fstream>

using namespace sdsl;
using namespace std;
using namespace std::chrono;

template<class t_cst>
void use_template(const char *textFileName, const char *patternFileName)
{
    int numberPattern = 0;
    int *lengthPattern = NULL;
    string *pattern = NULL;
    int i = 0;
    high_resolution_clock::time_point preprocessing_start;
    high_resolution_clock::time_point preprocessing_end;
    high_resolution_clock::time_point searching_start;
    high_resolution_clock::time_point searching_end;

    // open the file of patterns
    ifstream inputPatternFile(patternFileName, ios::in | ios::binary);
    if (!inputPatternFile)
    {
        // if open is failed, stop
        cout << "Cannot read the pattern file: " << patternFileName << endl;
        return;
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
	
    // open the file of patterns
    ifstream in(textFileName, ios::in | ios::binary);
	
    string text;
    int lengthText = 0;
    t_cst cst;

    if (!in)
    {
        // if open is failed, stop
        cout << "ERROR: cannot read the text file (\"" << textFileName << "\")" << endl;
        return;
    }
    // obtain length of text
    in.seekg(0, ios::end);
    lengthText = in.tellg();
    in.seekg(0, ios::beg);
    text.resize(lengthText);
    // store text into the string
    in.read(&text[0], lengthText);
    // close the file
    in.close();

    // construct CST (using CST-S algorithm)
    construct_im(cst, text, 1);

    size_t j = 0;

    // set the time stamp for the end of preprocessing
    preprocessing_end = high_resolution_clock::now();
    // set the time stamp for the start of searching
    searching_start = high_resolution_clock::now();

    for (i = 0; i < numberPattern; i++)
    {
        // the result is stored in occs
        auto occs = locate(cst, pattern[i].begin(), pattern[i].end());
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
}

int main (int argc, char **argv)
{
    // check the number of input arguments
    if (argc != 3)
    {
        // if number is incorrect, stop
        cout << "Usage: " << argv[0] << " [text file name] [pattern file name]" << endl;
        return 0;
    }

    // Use CST-S algorithm
    use_template<cst_sada<>>((const char *)argv[1], (const char *)argv[2]);

    return 0;
}
