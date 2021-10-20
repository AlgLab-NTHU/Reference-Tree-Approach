#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>

#define MAX_ALPHABET_SIZE 256
#define true 1
#define false 0

typedef unsigned long long ULL;
typedef char bool;
typedef struct _NODE
{
    int referenceString;
    int numberOfReference;
    bool order;
    char numberOfChildren;
    char *distance;
    struct _NODE **child;
} ReferenceTreeNode;
typedef struct _QUEUE
{
    void *pointerNode;
    int start;
    int numberOfInputString;
    bool order;
    struct _QUEUE *previous;
    struct _QUEUE *next;
} QueueNode;

int MyLog (int);
int ComputeHD_64 (ULL);
int ComputeHD_32 (ULL);

int MyLog (int input)
{
    int counter = 1;
    int result = 2;

    while (result < input)
    {
        result *= 2;
        counter++;
    }

    return counter;
}

int ComputeHD_64 (ULL i)
{
    i = i - ((i >> 1) & 0x5555555555555555UL);
    i = (i & 0x3333333333333333UL) + ((i >> 2) & 0x3333333333333333UL);
    return (int)((((i + (i >> 4)) & 0xF0F0F0F0F0F0F0FUL) * 0x101010101010101UL) >> 56);
}

int ComputeHD_32 (ULL i)
{
    i = i - ((i >> 1) & 0x55555555UL);
    i = (i & 0x33333333UL) + ((i >> 2) & 0x33333333UL);
    return (int)((((i + (i >> 4)) & 0xF0F0F0FUL) * 0x1010101UL) >> 24);
}

int main (int argc, char **argv)
{
    FILE *inputFile;
    char **pattern;
    int *lengthOfPattern;
    int numberOfPattern;
    int numberOfAlphabet;
    int parameterL;
    int parameterK;
    int i;
    int j;
    int k;
    int HD;
    int minimumLengthOfPattern;
    struct timeval preprocessingStart;   /* Time stamp for the start of preprocecssing */
    struct timeval preprocessingEnd;     /* Time stamp for the end of preprocessing */
    struct timeval searchingStart;       /* Time stamp for the start of searching */
    struct timeval searchingEnd;         /* Time stamp for the end of searching */


    if (argc != 5)
    {
        fprintf (stderr, "This program is to solve the multiple exact matching problem.\n");
        fprintf (stderr, "Usage: %s [text file] [pattern file] [parameter l] [parameter k]\n", argv[0]);

        return 0;
    }

    if ((inputFile = fopen (argv[2], "rb")) == NULL)
    {
        fprintf (stderr, "Error! Cannot read the pattern file %s\n", argv[2]);
        return 0x0003;
    }
    fscanf (inputFile, "%d\n", &numberOfPattern);
    lengthOfPattern = (int *)malloc (sizeof (int) * numberOfPattern);
    if (lengthOfPattern == NULL)
    {
        fprintf (stderr, "Error! Cannot request memory for recording the lengths of patterns!");
        return 0x0004;
    }
    pattern = (char **)malloc (sizeof (char *) * numberOfPattern);
    if (pattern == NULL)
    {
        fprintf (stderr, "Error! Cannot request memory for the first dimension of pattern!");
        free (lengthOfPattern);
        return 0x0005;
    }

    fscanf (inputFile, "%d,", &lengthOfPattern[0]);
    pattern[0] = malloc (lengthOfPattern[0] + 1);
    if (pattern[0] == NULL)
    {
        fprintf (stderr, "Error! Cannot request the memory for the 0th pattern array!");
        free (lengthOfPattern);
        free (pattern);
        return 0x0006;
    }
    fread (&pattern[0][0], 1, lengthOfPattern[0], inputFile);
    minimumLengthOfPattern = lengthOfPattern[0];
    fseek (inputFile, 1, SEEK_CUR);
    for (i = 1;i != numberOfPattern; i++)
    {
        fscanf (inputFile, "%d,", &lengthOfPattern[i]);
        pattern[i] = malloc (lengthOfPattern[i]);
        if (pattern[i] == NULL)
        {
            fprintf (stderr, "Error! Cannot request memory for the %dth pattern array!", i);
            free (lengthOfPattern);
            for (j = 0; j != i; j++)
            {
                free (pattern[j]);
            }
            free (pattern);
            return 0x0006;
        }
        fread (&pattern[i][0], 1, lengthOfPattern[i], inputFile);
        if (lengthOfPattern[i] < minimumLengthOfPattern)
        {
            minimumLengthOfPattern = lengthOfPattern[i];
        }
        fseek (inputFile, 1, SEEK_CUR);
    }
    fclose (inputFile);

    parameterL = atoi (argv[3]);
    parameterK = atoi (argv[4]);

    if (parameterL < minimumLengthOfPattern)
    {
        fprintf (stderr, "Error! The input parameter l is smaller than the minimum length among patterns!\n");
        free (lengthOfPattern);
        for (i = 0; i != numberOfPattern; i++)
        {
            free (pattern[i]);
        }
        free (pattern);

        return 0x0007;
    }

    gettimeofday(&preprocessingStart, NULL);

    char alphabet[MAX_ALPHABET_SIZE];
    char *text;
    int lengthOfText;

    int numberOfBitForSymbol;
    int numberOfSubstring;
    ULL cleanLeftBitMask;
    ULL *bitString;
    ULL bw;
    ULL bwbar;
    ULL bbase;
    ULL bitPattern;
    bool needShiftFlag;
    int *positionTemp[2];
    char *distance;
    int sizeOfCounting;
    int *counting;
    ReferenceTreeNode *root;
    ReferenceTreeNode *currentNode;
    QueueNode *head;
    QueueNode *currentJob;
    int start;
    int end;
    ULL reference;
    int numberOfInputString;
    int children;
    bool order;
    bool newOrder;
    bool flag;

    if ((inputFile = fopen (argv[1], "rb")) == NULL)
    {
        fprintf (stderr, "Error! Cannot read the text file %s!\n", argv[1]);
        return 0x0001;
    }
    memset (alphabet, MAX_ALPHABET_SIZE - 1, MAX_ALPHABET_SIZE);
    fseek (inputFile, 0, SEEK_END);
    lengthOfText = ftell (inputFile);
    fseek (inputFile, 0, SEEK_SET);
    text = (char *)malloc (lengthOfText + 1);
    if (text == NULL)
    {
        fprintf (stderr, "Error! Cannot request the memory for recording text string!\n");
        return 0x0002;
    }
    fread (&text[1], 1, lengthOfText, inputFile);
    fclose (inputFile);
    for (i = 1; i <= lengthOfText; i++)
    {
        alphabet[(unsigned char)text[i]] = 0;
    }

    numberOfAlphabet = 0;
    for (i = 0; i != MAX_ALPHABET_SIZE; i++)
    {
        if (alphabet[i] == 0)
        {
            alphabet[i] = numberOfAlphabet++;
        }
    }

    numberOfBitForSymbol = 1 + MyLog (numberOfAlphabet);
    if (sizeof(ULL) * 8 < parameterL * numberOfBitForSymbol)
    {
        fprintf (stderr, "Error! The size of bit string is greater than that of a computer word (w = %d)!\n", (int)sizeof (ULL) * 8);
        free (text);
        free (lengthOfPattern);
        for (i = 0; i != numberOfPattern; i++)
        {
            free (pattern[i]);
        }
        free (pattern);

        return 0x0008;
    }

    numberOfSubstring = lengthOfText - parameterL + 1;
    bitString = (ULL *)malloc (sizeof (ULL) * (numberOfSubstring + 1));
    //memset (bitString, 0, sizeof (ULL) * (numberOfSubstring + 1));

    if (parameterL * numberOfBitForSymbol == sizeof(ULL) * 8)
    {
        cleanLeftBitMask = ULLONG_MAX;
        needShiftFlag = false;
    }
    else
    {
        cleanLeftBitMask = 1UL;
        needShiftFlag = true;
    }
    // Initialize the temporary array for swap memory
    positionTemp[0] = (int *)malloc (sizeof (int) * (lengthOfText - parameterL + 2));
    positionTemp[1] = (int *)malloc (sizeof (int) * (lengthOfText - parameterL + 2));
    bbase = 1UL << (numberOfBitForSymbol - 1);
    bw = 0;
    bwbar = 0;
    bitString[1] = 0ULL;
    // For the bit string of s1
    for (i = 1; i <= parameterL; i++)
    {
        bitString[1] = (bitString[1] << numberOfBitForSymbol) | alphabet[(unsigned char)text[i]];

        bw = (bw << numberOfBitForSymbol) | bbase;
        bwbar = (bwbar << numberOfBitForSymbol) | (bbase - 1UL);

        if (needShiftFlag)
        {
            cleanLeftBitMask = cleanLeftBitMask << numberOfBitForSymbol;
        }
    }
    if (needShiftFlag)
    {
        cleanLeftBitMask = cleanLeftBitMask - 1UL;
    }

    positionTemp[0][1] = 1;
    // For the rest bit strings
    for (i = 2; i <= numberOfSubstring; i++)
    {
        positionTemp[0][i] = i;
        bitString[i] = ((bitString[i - 1] << numberOfBitForSymbol) | alphabet[(unsigned char)text[i + parameterL - 1]]) & cleanLeftBitMask;
    }

    distance = (char *)malloc (lengthOfText - parameterL + 2);
    sizeOfCounting = sizeof (int) * (parameterL + 1);
    counting = (int *)malloc (sizeOfCounting);

    root = (ReferenceTreeNode *)malloc (sizeof (ReferenceTreeNode));
    head = (QueueNode *)malloc (sizeof (QueueNode));
    head->start = 1;
    head->numberOfInputString = lengthOfText - parameterL + 1;
    head->order = 0;
    head->pointerNode = (void *)root;
    head->previous = NULL;
    
    // Start the construction of reference tree
    currentJob = head;
    while (currentJob != NULL)
    {
        // Copy the setting of the current job
        start = currentJob->start;
        numberOfInputString = currentJob->numberOfInputString;
        order = currentJob->order;
        currentNode = (ReferenceTreeNode *)(currentJob->pointerNode);

        // No other job exists
        if (currentJob->previous == NULL)
        {
            free (currentJob);
            currentJob = NULL;
        }
        else
        {
            currentJob = currentJob->previous;
            free (currentJob->next);
        }

        if (numberOfInputString > parameterK)
        {
            end = start + numberOfInputString;
            reference = bitString[positionTemp[(unsigned char)order][start]];

            memset (counting, 0, sizeOfCounting);
            for (i = start; i != end; i++)
            {
                HD = (char)ComputeHD_64 (((bitString[positionTemp[(unsigned char)order][i]] ^ reference) + bwbar) & bw);
                distance[i] = HD;
                counting[(unsigned char)HD]++;
            }

            children = 0;
            for (i = 1; i <= parameterL; i++)
            {
                if (counting[i]) children++;
                counting[i] += counting[i - 1];
            }

            if (children == 0)
            {
                currentNode->numberOfReference = numberOfInputString;
                currentNode->referenceString = start;
                currentNode->order = order;
                currentNode->numberOfChildren = 0;

                continue;
            }

            newOrder = (order + 1)%2;
            for (i = end - 1; i >= start; i--)
            {
                positionTemp[(unsigned char)newOrder][start + (--counting[(unsigned char)distance[i]])] = positionTemp[(unsigned char)order][i];
            }

            currentNode->numberOfReference = counting[1];
            currentNode->referenceString = start;
            currentNode->order = newOrder;
            currentNode->numberOfChildren = children + 1;
            currentNode->distance = (char *)malloc (children);
            currentNode->child = (ReferenceTreeNode **)malloc (sizeof (ReferenceTreeNode*) * children);

            for (i = 1, j = 0; i <= parameterL; i++)
            {
                k = ((i == parameterL ) ? (numberOfInputString - counting[i]) : (counting[i + 1] - counting[i]));
                if (!k)
                {
                    continue;
                }
                currentNode->distance[j] = i;
                currentNode->child[j] = (ReferenceTreeNode *)malloc (sizeof (ReferenceTreeNode));

                if (currentJob == NULL)
                {
                    currentJob = (QueueNode *) malloc (sizeof (QueueNode));
                    currentJob->previous = NULL;
                }
                else
                {
                    currentJob->next = (QueueNode *)malloc (sizeof (QueueNode));
                    currentJob->next->previous = currentJob;
                    currentJob = currentJob->next;
                }

                currentJob->pointerNode = (void *) (currentNode->child[j++]);
                currentJob->start = start + counting[i];
                currentJob->numberOfInputString = k;
                currentJob->order = newOrder;
            }
        }
        else
        {
            currentNode->numberOfReference = numberOfInputString;
            currentNode->referenceString = start;
            currentNode->order = order;
            currentNode->numberOfChildren = 0;
        }
    }

    gettimeofday(&preprocessingEnd, NULL);

    gettimeofday(&searchingStart, NULL);

    for (i = 0; i != numberOfPattern; i++)
    {
        bitPattern = 0ULL;
        for (j = 0; j != parameterL; j++)
        {
            bitPattern = (bitPattern << numberOfBitForSymbol) | alphabet[(unsigned char)pattern[i][j]];
        }

        currentNode = root;
        while (1)
        {
            if (currentNode->numberOfChildren != 0)
            {
                HD = (char)ComputeHD_64 (((bitPattern ^ bitString[positionTemp[(unsigned char)(currentNode->order)][currentNode->referenceString]]) + bwbar) & bw);
                if (HD)
                {
                    k = currentNode-> numberOfChildren - 1;
                    flag = false;
                    for (j = 0; j != k; j++)
                    {
                        if (currentNode->distance[j] == HD)
                        {
                            flag = true;
                            break;
                        }
                    }
                    if (flag == true)
                    {
                        currentNode = currentNode->child[j];
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    end = currentNode->referenceString + currentNode->numberOfReference;
                    order = currentNode->order;

                    if (parameterL == lengthOfPattern[i])
                    {
                        for (j = currentNode->referenceString; j != end; j++)
                        {
                            printf ("%d(%d),", positionTemp[(unsigned char)order][j] + parameterL - 1, i + 1);
                        }
                    }
                    else
                    {
                        for (start = currentNode->referenceString; start != end; start++)
                        {
                            flag = true;
                            for (j = parameterL, k = positionTemp[(unsigned char)order][start] + parameterL; j < lengthOfPattern[i]; j++, k++)
                            {
                                if (pattern[i][j] != text[k])
                                {
                                    flag = false;
                                    break;
                                }
                            }
                            if (flag == true)
                            {
                                if (k <= lengthOfText)
                                {
                                    printf ("%d(%d),", k - 1, i + 1);
                                }
                            }
                        }
                    }
                    break;
                }
            }
            else
            {
                end = currentNode->referenceString + currentNode->numberOfReference;
                order = currentNode->order;

                for (start = currentNode->referenceString; start != end; start++)
                {
                    flag = true;
                    for (j = 0, k = positionTemp[(unsigned char)order][start]; j < lengthOfPattern[i]; j++, k++)
                    {
                        if (pattern[i][j] != text[k])
                        {
                            flag = false;
                            break;
                        }
                    }
                    if (flag == true)
                    {
                        if (k <= lengthOfText)
                        {
                            printf ("%d(%d),", k - 1, i + 1);
                        }
                    }
                }
                break;
            }
        }
    }
    gettimeofday(&searchingEnd, NULL);

    printf("\t%lf\t%lf\t%lf\n", (preprocessingEnd.tv_sec - preprocessingStart.tv_sec) + (preprocessingEnd.tv_usec - preprocessingStart.tv_usec)/1000000.0, (searchingEnd.tv_sec - searchingStart.tv_sec) + (searchingEnd.tv_usec - searchingStart.tv_usec)/1000000.0, (searchingEnd.tv_sec - preprocessingStart.tv_sec) + (searchingEnd.tv_usec - preprocessingStart.tv_usec)/1000000.0);

    free (text);
    free (lengthOfPattern);
    for (i = 0; i != numberOfPattern; i++)
    {
        free (pattern[i]);
    }
    free (pattern);
    free (bitString);
    free (positionTemp[0]);
    free (positionTemp[1]);
    free (distance);

    return 0;
}
