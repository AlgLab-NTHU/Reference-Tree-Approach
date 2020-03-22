#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* Define the maximum size of alphabet */
#define MAX_ALPHABET_SIZE 128
/* Define boolean values */
#define true 1
#define false 0

/* Define the abbreviated names */
typedef unsigned long long ULL;
typedef char bool;

/* Define the struct for a node in the reference tree */
/* Note that this struct is for internal and leaf nodes */
typedef struct _REFERENCE_TREE_NODE
{
    int number_reference;                     /* The number of input strings */
    int *reference;                           /* The reference string */
    ULL *bit_vector_reference;                /* The bit string with respect to reference string */
    char children;                            /* The number of child nodes (which will be expanded)*/
    char *distance;                           /* The child nodes' distances (with the reference string) */
    struct _REFERENCE_TREE_NODE **child_node; /* pointers for child nodes */
} ReferenceTreeNode;

/* Define the struct for an element in job stack */
typedef struct _STACK
{
    int start;                                /* The start position for the memory */
    int number_input_string;                  /* The number of input strings */
    bool order;                               /* The flag for accessing memory */
    struct _REFERENCE_TREE_NODE *node;        /* The child node needed to be expaned */
    struct _STACK *previous;                  /* The previous element in the job stack */
    struct _STACK *next;                      /* The next element in the job stack */
} Stack;

int ComputeHammingDistance_32bits(ULL i);
int ComputeHammingDistance_64bits(ULL i);
void FreeTree(ReferenceTreeNode **node);

/* Calculate the number of bits whose values are 1 in a computer word for 32-bit operating system */
int ComputeHammingDistance_32bits(ULL i)
{
    i = i - ((i >> 1) & 0x55555555UL);
    i = (i & 0x33333333UL) + ((i >> 2) & 0x33333333UL);
    return (int)((((i + (i >> 4)) & 0x0F0F0F0FUL) * 0x01010101UL) >> 24);
}

/* Calculate the number of bits whose values are 1 in a computer word for 64-bit operating system */
int ComputeHammingDistance_64bits(ULL i)
{
    i = i - ((i >> 1) & 0x5555555555555555UL);
    i = (i & 0x3333333333333333UL) + ((i >> 2) & 0x3333333333333333UL);
    return (int)((((i + (i >> 4)) & 0xF0F0F0F0F0F0F0FUL) * 0x101010101010101UL) >> 56);
}

/* Travel and print the reference tree by DFS */
void TravelTree(ReferenceTreeNode *node)
{
    int i = 0;

    if (node->children != 0)
    {
        printf("reference = ");
        for (i = 0; i < node->number_reference; i++)
            printf("%d,", node->reference[i]);
        printf("(%d) -> %llu\n", node->number_reference, node->bit_vector_reference[0]);
        for (i = 0; i < node->children - 1; i++)
            TravelTree(node->child_node[i]);
    }
    else
    {
        for (i = 0; i < node->number_reference; i++)
            printf("%d,", node->reference[i]);
        printf("\n");
    }
}

/* Free the memory of the node in the reference tree */
void FreeTree(ReferenceTreeNode **node)
{
    if ((*node)->children > 1)
    {
        int i = 0;
        for (;i < (*node)->children - 1; i++)
            FreeTree(&((*node)->child_node[i]));
        free((*node)->child_node);
        free((*node)->distance);
        free((*node)->bit_vector_reference);
        free((*node)->reference);
    }
    else if ((*node)->children == 1)
    {
        free((*node)->bit_vector_reference);
        free((*node)->reference);
    }
    else
    {
        free((*node)->reference);
    }

    free(*node);
}

int main(int argc, char **argv)
{
    char *text = NULL;                       /* The array for recording text */
    int length_text = 0;                     /* The length of text */
    char **pattern = NULL;                   /* The array for recording patterns */
    int number_pattern = 0;                  /* The number of patterns */
    int *length_pattern = NULL;              /* The array for recording lengths of pattern */
    int lmin = 0;                            /* Recording the mininum length among all patterns */
    FILE *input_text_file = NULL;            /* The FILE pointer for opening the file of text */
    FILE *input_pattern_file = NULL;         /* The FILE pointer for opening the file of patterns */
    int parameter_l = 0;                     /* Parameter l */
    int parameter_k = 0;                     /* Parameter k */
    int i = 0;                               /* for loop index */
    struct timeval preprocessing_start = {}; /* Time stamp for the start of preprocecssing */
    struct timeval preprocessing_end = {};   /* Time stamp for the end of preprocessing */
    struct timeval searching_start = {};     /* Time stamp for the start of searching */
    struct timeval searching_end = {};       /* Time stamp for the end of searching */

    /* Checking the number of input arguments */
    /* When the input number is incorrect, the help message for using this program is displayed. */
    if (argc != 5)
    {
        printf("The number of parameters is incorrect!\n");
        printf("Usage: %s [text file] [pattern file] [parameter l] [parameter k]\n", argv[0]);
        printf("       [text file]: the file name of the input text string\n");
        printf("       [pattern file]: the file name of the input pattern strings\n");
        printf("       [parameter l]: the user's assigned length of prefixes\n");
        printf("       [parameter k]: the user's assigned size for determining whether a node is a leaf in reference tree\n\n");
    }

    /* Checking whether the opening file for patterns is successfully */
    if ((input_pattern_file = fopen(argv[2], "rb")) == NULL)
    {
        /* If fail to open the file of patterns, stop this program */
        printf("[Error]: Cannot read the pattern file \"%s\"\n", argv[2]);
        return 0;
    }

    /* Note that the format of patterns is described as follows:
     * The first line is the total number of patterns.
     * Patterns are recorded in the rest of lines.
     * Each line (from 2nd to the last lines) contains two elements.
     * The length is recorded in the first element, and its corresponding pattern is in the second element.
     * Two elements are separated by a comma.
     * */
    /* Read the number of pattern */
    fscanf(input_pattern_file, "%d\n", &number_pattern);
    /* Request memory for the first dimension of pattern array */
    pattern = (char **)malloc(sizeof(char *) * number_pattern);
    /* Request memory for the lengths of patterns */
    length_pattern = (int *)malloc(sizeof(int) * number_pattern);
    /* Read each line to obtain the length and the pattern by for loop */
    for (i = 0; i < number_pattern; i++)
    {
        /* Obtain the length of the pattern first, and then request the memory of this pattern array (the second dimension) */
        fscanf(input_pattern_file, "%d%*c", &length_pattern[i]);
        /* Record the minimum length among all patterns */
        if (i == 0)
            lmin = length_pattern[i];
        else if (lmin > length_pattern[i])
            lmin = length_pattern[i];
        pattern[i] = (char *)malloc(sizeof(char) * (length_pattern[i] + 1));
        /* Read the pattern and store into pattern array */
        fread(&pattern[i][0], sizeof(char), length_pattern[i], input_pattern_file);
        /* Prepare for next line in the file */
        fscanf(input_pattern_file, "%*c");
    }
    /* close the file of patterns */
    fclose(input_pattern_file);

    /* obtain the parameters l and k */
    parameter_l = atoi(argv[3]);
    parameter_k = atoi(argv[4]);

    /* set the time stamp of the start for preprocessing */
    gettimeofday(&preprocessing_start, NULL);

    /* Check whether the opening file for text is successful */
    if ((input_text_file = fopen(argv[1], "rb")) == NULL)
    {
        /* If fail to open the file, free the used memory and stop. */
        printf("[Error]: Cannot read the text file \"%s\"\n", argv[1]);
        for (i = 0; i < number_pattern; i++)
            free(pattern[i]);
        free(pattern);
        return 0;
    }
    /* Obtain the length of text string */
    fseek(input_text_file, 0, SEEK_END);
    length_text = ftell(input_text_file);
    fseek(input_text_file, 0, SEEK_SET);
    /* Request memory for text string */
    text = (char *)malloc(sizeof(char) * (length_text + 1));
    /* Store text string into array */
    fread(&text[1], sizeof(char), length_text, input_text_file);
    /* Close the file of text */
    fclose(input_text_file);

    int alphabet[MAX_ALPHABET_SIZE] = {};       /* The alphabet of strings */
    int size_alphabet = 0;                      /* The size of alphabet */
    char ENV = 64;                              /* The size of a computer word */
    char number_bit_for_a_symbol = 1;           /* The number of bits used to record a symbol */
    char number_ull_for_a_string = 1;           /* The number of ULLs that an input string needs */
    char number_symbol_for_a_ull = 1;           /* The number of symbols can be recorded in a ULL for text string */
    char number_symbol_for_last_ull = 1;        /* The number of symbols recording in the last ULL for text string */
    char highest_position_in_a_ull = 1;         /* The highest position of the bit in last ULL that we will use */
    char highest_position_in_last_ull = 1;      /* The highest position of the bit in a ULL (excepted the last ULL) that we will use */
    ULL filter_usless_bit_in_a_ull = 0x1ULL;    /* The mask for filtering unused bits in a ULL */
    ULL filter_usless_bit_in_last_ull = 0x1ULL; /* The mask for filtering unused bits in the last ULL */
    ULL keep_bit_for_witness = 0x0ULL;          /* The mask for filtering the bits whic are not witness bits */
    ULL clear_bit_for_witness = 0x0ULL;         /* The mask for filtering the witness bits */
    ULL auxiliary_ull = 0x0ULL;                 /* A temporary ULL */
    ULL *bit_vector_for_text = NULL;            /* The bit string with respect to text string */
    ULL *temp_bit_vector = NULL;                /* The bit strings table for all input strings */
    int number_input_string = 0;                /* The number of input strings */
    int *input_string = NULL;                   /* The input strings */
    int *sort_string = NULL;                    /* The strings sorted by their positions */
    char *distance = NULL;                      /* The Hamming distances of child nodes */
    int *counting = NULL;                       /* The counting table for counting sort */
    int size_of_ull_string = 0;                 /* The size of an input string (the unit is byte) */
    int start = 0;                              /* The start position in memory for input string */
    int end = 0;                                /* The end position in memory for input string */
    bool order = true;                          /* The flag for accessing memory */
    ReferenceTreeNode *root = NULL;             /* Root node pointer */
    ReferenceTreeNode *current_node = NULL;     /* The node pointer which is ready to be expanded */
    Stack *head = NULL;                         /* The head pointer of the stack */
    Stack *current_job = NULL;                  /* The current pointer which is ready to be processing */
    int reference = 0;                          /* The position for reference string */
    char children = 0;                          /* The number of child nodes */
    int HD = 0;
    int j = 0;
    int k = 0;

    /* scan the text and record the alphabet of text */
    memset(alphabet, -128, MAX_ALPHABET_SIZE);
    for (i = 1; i <= length_text; i++)
        alphabet[(int)text[i]] = 1;

    /* Assign an unique number for this symbol */
    for (i = 0; i < MAX_ALPHABET_SIZE; i++)
        if (alphabet[i] == 1)
            alphabet[i] = size_alphabet++;

    /* obtain the size of a computer word */
    ENV = sizeof(ULL) * 8;
    /* Calculate the number of bits used to record a symbol */
    for (i = 2; i < size_alphabet; i *= 2)
        number_bit_for_a_symbol = number_bit_for_a_symbol + 1;
    /* An extra leading witness bit is needed */
    number_bit_for_a_symbol = number_bit_for_a_symbol + 1;
    /* Compute the maximum number of characters in an ULL */
    number_symbol_for_a_ull = ENV/number_bit_for_a_symbol;

    /* Compute the number of ULLs that text string needs */
    if (parameter_l % number_symbol_for_a_ull)
    {
        number_ull_for_a_string = parameter_l / number_symbol_for_a_ull + 1;
        number_symbol_for_last_ull = parameter_l % number_symbol_for_a_ull;
    }
    else
    {
        number_ull_for_a_string = parameter_l / number_symbol_for_a_ull;
        number_symbol_for_last_ull = number_symbol_for_a_ull;
    }

    /* obtain the highest position of bits in a ULL (and the last ULL) of the bit string with respect to text */
    highest_position_in_last_ull = number_symbol_for_last_ull * number_bit_for_a_symbol;
    highest_position_in_a_ull = (number_ull_for_a_string == 1) ? highest_position_in_last_ull : number_symbol_for_a_ull * number_bit_for_a_symbol;

    /* If characters can exactly fill up in an ULL, no bit will be filted;
     * Otherwise, compute the mask for filtering the unused bits in the high positions.
     * */
    if (ENV == highest_position_in_a_ull)
        filter_usless_bit_in_a_ull = ~0x0ULL;
    else
        filter_usless_bit_in_a_ull = (0x1ULL << highest_position_in_a_ull) - 0x1ULL;
    if (number_symbol_for_last_ull == number_symbol_for_a_ull)
        filter_usless_bit_in_last_ull = filter_usless_bit_in_a_ull;
    else
    {
        if (ENV == highest_position_in_last_ull)
            filter_usless_bit_in_last_ull = ~0x0ULL;
        else
            filter_usless_bit_in_last_ull = (0x1ULL << highest_position_in_last_ull) - 0x1ULL;
    }

    /* Compute the masks for filtering witness bits and keeping witness bits */
    keep_bit_for_witness = auxiliary_ull = 0x1ULL << (number_bit_for_a_symbol - 1);
    for (i = 1; i < number_symbol_for_a_ull; i++)
        keep_bit_for_witness = (keep_bit_for_witness << number_bit_for_a_symbol) | auxiliary_ull;
    clear_bit_for_witness = (~keep_bit_for_witness) & filter_usless_bit_in_a_ull;

    /* Request memory for bit string with respect to text string */
    number_input_string = length_text - (lmin > parameter_l ? lmin : parameter_l) + 1;
    bit_vector_for_text = (ULL *)malloc(sizeof(ULL) * number_input_string * number_symbol_for_a_ull);
    /* Request memory for a temporary bit string which is equal to that of whole text */
    temp_bit_vector = (ULL *)malloc(sizeof(ULL) * number_input_string * number_symbol_for_a_ull);
    /* Request memory for input strings */
    input_string = (int *)malloc(sizeof(int) * number_input_string);
    /* Request memory for the strings which will be sorted */
    sort_string = (int *)malloc(sizeof(int) * number_input_string);
    /* Request memory for recording the distances between input string and reference string */
    distance = (char *)malloc(sizeof(char) * number_input_string);
    /* Request memory for the table of counting sort */
    counting = (int *)malloc(sizeof(int) * (parameter_l + 1));

    /* Scan text string and transfer each character into its corresponding number
     * The first step is to compute the bit string with respect to the first input string whose position is the smallest in the text.
     * After obtaining the first bit string, the bit string of the next input string can be computed by the first.
     * By the same method, all bit strings can be computed one by one.
     * */
    /* Compute the bit string of the first input string of text */
    for (i = 0; i < number_ull_for_a_string - 1; i++)
    {
        for (j = 1; j <= number_symbol_for_a_ull; j++)
            bit_vector_for_text[i] = ((bit_vector_for_text[i] << number_bit_for_a_symbol) | alphabet[(int)text[i * number_symbol_for_a_ull + j]]);
        bit_vector_for_text[i] = bit_vector_for_text[i] & filter_usless_bit_in_a_ull;
    }
    for (j = 1; j <= number_symbol_for_last_ull; j++)
        bit_vector_for_text[i] = ((bit_vector_for_text[i] << number_bit_for_a_symbol) | alphabet[(int)text[i * number_symbol_for_a_ull + j]]);
    bit_vector_for_text[i] = bit_vector_for_text[i] & filter_usless_bit_in_last_ull;

    /* Obtain the size of a input string (the unit is byte) */
    size_of_ull_string = sizeof(ULL) * number_ull_for_a_string;

    /* Compute the rest of bit strings of input strings of text */
    input_string[0] = 1;
    for (i = 1; i < number_input_string; i++)
    {
       input_string[i] = i + 1;
       memcpy(&bit_vector_for_text[i * number_ull_for_a_string], &bit_vector_for_text[(i - 1) * number_ull_for_a_string], size_of_ull_string);
       if (number_ull_for_a_string == 1)
       {
           k = i + number_symbol_for_last_ull;
           j = 0;
       }
       else
       {
           for (j = 0, k = i + number_symbol_for_a_ull; j < number_ull_for_a_string - 1; j++, k += number_symbol_for_a_ull)
               bit_vector_for_text[i * number_ull_for_a_string + j] = ((bit_vector_for_text[i * number_ull_for_a_string + j] << number_bit_for_a_symbol) | alphabet[(int)text[k]]) & filter_usless_bit_in_a_ull;
           k = k - number_symbol_for_a_ull + number_symbol_for_last_ull;
       }
       bit_vector_for_text[i * number_ull_for_a_string + j] = ((bit_vector_for_text[i * number_ull_for_a_string + j] << number_bit_for_a_symbol) | alphabet[(int)text[k]]) & filter_usless_bit_in_last_ull;
    }

    /* Request memory for root node */
    root = (ReferenceTreeNode *)malloc(sizeof(ReferenceTreeNode));
    /* Request memory for the first element of stack and then initialize it */
    head = (Stack *)malloc(sizeof(Stack));
    head->start = 0;
    head->number_input_string = number_input_string;
    head->order = true;
    head->node = root;
    head->previous = NULL;
    head->next = NULL;
    /* Point the first element of stack to be the element ready to be processing */
    current_job = head;

    /* Use 64-bit scheme to deal with the preprocessing */
    if (ENV == 64)
    {
        /* Dealing with the current element */
        while (current_job != NULL)
        {
            /* Dealing with the current element */
            start = current_job->start;
            number_input_string = current_job->number_input_string;
            order = current_job->order;
            current_node = current_job->node;

             /* Point to next element */
            if (current_job->previous == NULL)
            {
                free(current_job);
                current_job = NULL;
            }
            else
            {
                current_job = current_job->previous;
                free(current_job->next);
                current_job->next = NULL;
            }

            /* Consider the number of input strings for the current node.
             * If the number is greater than parameter k, this node is an internal node needed to be expanded.
             * Otherwise, this node is a leaf node.
             * */
            if (number_input_string > parameter_k) /* Internal node */
            {
                if (order == true) /* If the flag is true, the access memory is the bit_bector_for_text. */
                {
                    /* Calculate the ending position in the memory */
                    end = start + number_input_string;

                    /* Pick the first element to be reference string */
                    reference = start;
                    /* Initialize the table for counting sort */
                    memset(counting, 0, sizeof(int) * (parameter_l + 1));
                    /* Compute the Hamming distances between each input string and the reference string */
                    for (i = start; i != end; i++)
                    {
                        HD = 0;
                        for (j = 0; j < number_ull_for_a_string; j++) /* popcount method for computing Hamming distance applying 64-bit scheme */
                            HD += ComputeHammingDistance_64bits(((bit_vector_for_text[i * number_ull_for_a_string + j] ^ bit_vector_for_text[reference * number_ull_for_a_string + j]) + clear_bit_for_witness) & keep_bit_for_witness);
                        distance[i] = HD;
                        counting[HD]++;
                    }

                    /* Comuting the number of child nodes and update the table of counting sort */
                    children = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        if (counting[i] != 0)
                            children = children + 1;
                        counting[i] += counting[i - 1];
                    }

                    current_node->children = children + 1;
                    /* If all input string are identical, this node becomes leaf node */
                    if (children == 0)
                    {
                        current_node->number_reference = number_input_string;
                        current_node->reference = (int *)malloc(sizeof(int) * number_input_string);
                        memcpy(&current_node->reference[0], &input_string[start], sizeof(int) * number_input_string);
                        current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                        memcpy(&current_node->bit_vector_reference[0], &bit_vector_for_text[reference * number_ull_for_a_string], size_of_ull_string);

                        continue;
                    }

                    /* Update this internal node. */
                    /* Copy the memory from bit_vector_for_text to temp_bit_vector for the further expanding subtree. */
                    for (i = end - 1; i >= start; i--)
                    {
                        j = --counting[(int)distance[i]];
                        memcpy(&temp_bit_vector[(start + j) * number_ull_for_a_string], &bit_vector_for_text[i * number_ull_for_a_string], size_of_ull_string);
                        sort_string[start + j] = input_string[i];
                    }

                    /* 1. Record the number of reference string */
                    current_node->number_reference = counting[1];
                    /* 2. Keep all reference stirngs */
                    current_node->reference = (int *)malloc(sizeof(int) * counting[1]);
                    memcpy(&current_node->reference[0], &sort_string[start], sizeof(int) * counting[1]);
                    /* 3. The bit string with respect to the reference string is recorded in this node.
                     * This is the scheme that use a large table of all input strings for avoding the computing of extracting the bit string from whole text.
                     * */
                    current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                    memcpy(&current_node->bit_vector_reference[0], &bit_vector_for_text[reference * number_ull_for_a_string], size_of_ull_string);

                    /* 4. Request memory for child nodes (include their corresponding distance) */
                    current_node->distance = (char *)malloc(children);
                    current_node->child_node = (ReferenceTreeNode **)malloc(sizeof(ReferenceTreeNode *) * children);

                    /* Store the processing of expanding child nodes into stack */
                    j = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        k = (i == parameter_l) ? number_input_string - counting[i] : counting[i + 1] - counting[i];
                        if (k == 0)
                            continue;
                        /* Request memory for the pointer of child nodes and the corrersponding distances */
                        current_node->distance[j] = i;
                        current_node->child_node[j] = (ReferenceTreeNode *)malloc(sizeof(ReferenceTreeNode));

                        /* Request memory for this element of stack */
                        if (current_job == NULL)
                        {
                            current_job = (Stack *)malloc(sizeof(Stack));
                            current_job->previous = NULL;
                        }
                        else
                        {
                            current_job->next = (Stack *)malloc(sizeof(Stack));
                            current_job->next->previous = current_job;
                            current_job = current_job->next;
                        }

                        /* Sotre the relative data of this element into stack */
                        current_job->start = start + counting[i];
                        current_job->number_input_string = k;
                        current_job->order = false;
                        current_job->node = current_node->child_node[j++];
                    }
                }
                else /* If the flag is true, the access memory is the temp_bit_vector. The explanations please consider the above part (order=true)  */
                {
                    end = start + number_input_string;

                    reference = start;
                    memset(counting, 0, sizeof(int) * (parameter_l + 1));
                    for (i = start; i != end; i++)
                    {
                        HD = 0;
                        for (j = 0; j < number_ull_for_a_string; j++)
                            HD += ComputeHammingDistance_64bits(((temp_bit_vector[i * number_ull_for_a_string + j] ^ temp_bit_vector[reference * number_ull_for_a_string + j]) + clear_bit_for_witness) & keep_bit_for_witness);
                        distance[i] = HD;
                        counting[HD]++;
                    }

                    children = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        if (counting[i] != 0)
                            children = children + 1;
                        counting[i] += counting[i - 1];
                    }

                    current_node->children = children + 1;
                    if (children == 0)
                    {
                        current_node->number_reference = number_input_string;
                        current_node->reference = (int *)malloc(sizeof(int) * number_input_string);
                        memcpy(&current_node->reference[0], &sort_string[start], sizeof(int) * number_input_string);
                        current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                        memcpy(&current_node->bit_vector_reference[0], &temp_bit_vector[reference * number_ull_for_a_string], size_of_ull_string);

                        continue;
                    }

                    /* Update this internal node. */
                    /* Copy the memory from temp_bit_vector to bit_vector_for_text for the further expanding subtree. */
                    for (i = end - 1; i >= start; i--)
                    {
                        j = --counting[(int)distance[i]];
                        memcpy(&bit_vector_for_text[(start + j) * number_ull_for_a_string], &temp_bit_vector[i * number_ull_for_a_string], size_of_ull_string);
                        input_string[start + j] = sort_string[i];
                    }

                    current_node->number_reference = counting[1];
                    current_node->reference = (int *)malloc(sizeof(int) * counting[1]);
                    memcpy(&current_node->reference[0], &input_string[start], sizeof(int) * counting[1]);
                    current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                    memcpy(&current_node->bit_vector_reference[0], &temp_bit_vector[reference * number_ull_for_a_string], size_of_ull_string);

                    current_node->distance = (char *)malloc(children);
                    current_node->child_node = (ReferenceTreeNode **)malloc(sizeof(ReferenceTreeNode *) * children);

                    j = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        k = (i == parameter_l) ? number_input_string - counting[i] : counting[i + 1] - counting[i];
                        if (k == 0)
                            continue;
                        current_node->distance[j] = i;
                        current_node->child_node[j] = (ReferenceTreeNode *)malloc(sizeof(ReferenceTreeNode));

                        if (current_job == NULL)
                        {
                            current_job = (Stack *)malloc(sizeof(Stack));
                            current_job->previous = NULL;
                        }
                        else
                        {
                            current_job->next = (Stack *)malloc(sizeof(Stack));
                            current_job->next->previous = current_job;
                            current_job = current_job->next;
                        }
                        current_job->start = start + counting[i];
                        current_job->number_input_string = k;
                        current_job->order = true;
                        current_job->node = current_node->child_node[j++];
                    }
                }
            }
            else /* Leaf node */
            {
                /* Store the input strings, the start position in the memory, and the flag of accessing memory */
                current_node->children = 0;
                current_node->number_reference = number_input_string;
                current_node->reference = (int *)malloc(sizeof(int) * number_input_string);
                if (order == true)
                    memcpy(&current_node->reference[0], &input_string[start], sizeof(int) * number_input_string);
                else
                    memcpy(&current_node->reference[0], &sort_string[start], sizeof(int) * number_input_string);
            }        
        }
    }
    else /* Use 32-bit scheme to deal with the precessing. Please consider the explanations of the above 64-bit scheme. */
    {
        while (current_job != NULL)
        {
            start = current_job->start;
            number_input_string = current_job->number_input_string;
            order = current_job->order;
            current_node = current_job->node;

            if (current_job->previous == NULL)
            {
                free(current_job);
                current_job = NULL;
            }
            else
            {
                current_job = current_job->previous;
                free(current_job->next);
            }

            if (number_input_string > parameter_k)
            {
                if (order == true)
                {
                    end = start + number_input_string;

                    reference = start;
                    memset(counting, 0, sizeof(int) * (parameter_l + 1));
                    for (i = start; i != end; i++)
                    {
                        HD = 0;
                        for (j = 0; j < number_ull_for_a_string; j++)
                            HD += ComputeHammingDistance_32bits(((bit_vector_for_text[i * number_ull_for_a_string + j] ^ bit_vector_for_text[reference * number_ull_for_a_string + j]) + clear_bit_for_witness) & keep_bit_for_witness);
                        distance[i] = HD;
                        counting[HD]++;
                    }

                    children = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        if (counting[i] != 0)
                            children = children + 1;
                        counting[i] += counting[i - 1];
                    }

                    current_node->children = children + 1;
                    if (children == 0)
                    {
                        current_node->number_reference = number_input_string;
                        current_node->reference = (int *)malloc(sizeof(int) * number_input_string);
                        memcpy(&current_node->reference[0], &input_string[start], sizeof(int) * number_input_string);
                        current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                        memcpy(&current_node->bit_vector_reference[0], &bit_vector_for_text[reference * number_ull_for_a_string], size_of_ull_string);

                        continue;
                    }

                    for (i = end - 1; i >= start; i--)
                    {
                        j = --counting[(int)distance[i]];
                        memcpy(&temp_bit_vector[(start + j) * number_ull_for_a_string], &bit_vector_for_text[i * number_ull_for_a_string], size_of_ull_string);
                        sort_string[start + j] = input_string[i];
                    }

                    current_node->number_reference = counting[1];
                    current_node->reference = (int *)malloc(sizeof(int) * counting[1]);
                    memcpy(&current_node->reference[0], &sort_string[start], sizeof(int) * counting[1]);
                    current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                    memcpy(&current_node->bit_vector_reference[0], &bit_vector_for_text[reference * number_ull_for_a_string], size_of_ull_string);

                    current_node->distance = (char *)malloc(children);
                    current_node->child_node = (ReferenceTreeNode **)malloc(sizeof(ReferenceTreeNode *) * children);

                    j = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        k = (i == parameter_l) ? number_input_string - counting[i] : counting[i + 1] - counting[i];
                        if (k == 0)
                            continue;
                        current_node->distance[j] = i;
                        current_node->child_node[j] = (ReferenceTreeNode *)malloc(sizeof(ReferenceTreeNode));

                        if (current_job == NULL)
                        {
                            current_job = (Stack *)malloc(sizeof(Stack));
                            current_job->previous = NULL;
                        }
                        else
                        {
                            current_job->next = (Stack *)malloc(sizeof(Stack));
                            current_job->next->previous = current_job;
                            current_job = current_job->next;
                        }
                        current_job->start = start + counting[i];
                        current_job->number_input_string = k;
                        current_job->order = false;
                        current_job->node = current_node->child_node[j++];
                    }
                }
                else
                {
                    end = start + number_input_string;

                    reference = start;
                    memset(counting, 0, sizeof(int) * (parameter_l + 1));
                    for (i = start; i != end; i++)
                    {
                        HD = 0;
                        for (j = 0; j < number_ull_for_a_string; j++)
                            HD += ComputeHammingDistance_32bits(((temp_bit_vector[i * number_ull_for_a_string + j] ^ temp_bit_vector[reference * number_ull_for_a_string + j]) + clear_bit_for_witness) & keep_bit_for_witness);
                        distance[i] = HD;
                        counting[HD]++;
                    }

                    children = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        if (counting[i] != 0)
                            children = children + 1;
                        counting[i] += counting[i - 1];
                    }

                    current_node->children = children + 1;
                    if (children == 0)
                    {
                        current_node->number_reference = number_input_string;
                        current_node->reference = (int *)malloc(sizeof(int) * number_input_string);
                        memcpy(&current_node->reference[0], &sort_string[start], sizeof(int) * number_input_string);
                        current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                        memcpy(&current_node->bit_vector_reference[0], &temp_bit_vector[reference * number_ull_for_a_string], size_of_ull_string);

                        continue;
                    }

                    for (i = end - 1; i >= start; i--)
                    {
                        j = --counting[(int)distance[i]];
                        memcpy(&bit_vector_for_text[(start + j) * number_ull_for_a_string], &temp_bit_vector[i * number_ull_for_a_string], size_of_ull_string);
                        input_string[start + j] = sort_string[i];
                    }

                    current_node->number_reference = counting[1];
                    current_node->reference = (int *)malloc(sizeof(int) * counting[1]);
                    memcpy(&current_node->reference[0], &input_string[start], sizeof(int) * counting[1]);
                    current_node->bit_vector_reference = (ULL *)malloc(size_of_ull_string);
                    memcpy(&current_node->bit_vector_reference[0], &temp_bit_vector[reference * number_ull_for_a_string], size_of_ull_string);

                    current_node->distance = (char *)malloc(children);
                    current_node->child_node = (ReferenceTreeNode **)malloc(sizeof(ReferenceTreeNode *) * children);

                    j = 0;
                    for (i = 1; i <= parameter_l; i++)
                    {
                        k = (i == parameter_l) ? number_input_string - counting[i] : counting[i + 1] - counting[i];
                        if (k == 0)
                            continue;
                        current_node->distance[j] = i;
                        current_node->child_node[j] = (ReferenceTreeNode *)malloc(sizeof(ReferenceTreeNode));

                        if (current_job == NULL)
                        {
                            current_job = (Stack *)malloc(sizeof(Stack));
                            current_job->previous = NULL;
                        }
                        else
                        {
                            current_job->next = (Stack *)malloc(sizeof(Stack));
                            current_job->next->previous = current_job;
                            current_job = current_job->next;
                        }
                        current_job->start = start + counting[i];
                        current_job->number_input_string = k;
                        current_job->order = true;
                        current_job->node = current_node->child_node[j++];
                    }
                }
            }
            else
            {
                current_node->children = 0;
                current_node->number_reference = number_input_string;
                current_node->reference = (int *)malloc(sizeof(int) * number_input_string);
                if (order == true)
                    memcpy(&current_node->reference[0], &input_string[start], sizeof(int) * number_input_string);
                else
                    memcpy(&current_node->reference[0], &sort_string[start], sizeof(int) * number_input_string);
            }        
        }
    }

//    TravelTree(root);

    /* Prepare the memory for pattern */
    ULL *bit_vector_for_pattern = (ULL *)malloc(size_of_ull_string);
    bool search = true;

    /* Set the time stamp of the end for preprocessing */
    gettimeofday(&preprocessing_end, NULL);
    /* Set the time stamp of the start for searching */
    gettimeofday(&searching_start, NULL);

    /* Use 64-bit scheme to deal with the searching */
    if (ENV == 64)
    {
        /* Consider the patterns one by one */
        for (i = 0; i < number_pattern; i++)
        {
            /* Obtain the bit string with respect to the reference string */
            for (j = 0; j < number_ull_for_a_string - 1; j++)
                for (k = 0; k < number_symbol_for_a_ull; k++)
                    bit_vector_for_pattern[j] = ((bit_vector_for_pattern[j] << number_bit_for_a_symbol) | alphabet[(int)pattern[i][j * number_symbol_for_a_ull + k]]) & filter_usless_bit_in_a_ull;
            for (k = 0; k < number_symbol_for_last_ull; k++)
                bit_vector_for_pattern[j] = ((bit_vector_for_pattern[j] << number_bit_for_a_symbol) | alphabet[(int)pattern[i][j * number_symbol_for_a_ull + k]]) & filter_usless_bit_in_last_ull;

            /* Search the reference tree started from root node */
            current_node = root;
            search = true;
            while (search == true)
            {
                /* If this is an internal node, compute the Hamming distance between l-prefix of this pattern and the reference string */
                if (current_node->children != 0)
                {
                    HD = 0;
                    /* Compute the Hamming distance by popcount method */
                    for (j = 0; j < number_ull_for_a_string; j++)
                        HD += ComputeHammingDistance_64bits(((bit_vector_for_pattern[j] ^ current_node->bit_vector_reference[j]) + clear_bit_for_witness) & keep_bit_for_witness);

                    /* If the distance is zero, search go to the special leaf node whose strings all are identical to the l-prefix of this pattern */
                    if (HD == 0)
                    {
                        /* compare the suffixes of patterns with that of the string in the special leaf node (whose prefix is equal to the reference string */
                        number_input_string = current_node->number_reference;
                        if (parameter_l == length_pattern[i])
                        {
                            for (j = 0; j < number_input_string; j++)
                                printf("%d(%d),", current_node->reference[j] + parameter_l - 1, i + 1);
                        }
                        else
                        {
                            for (start = 0; start < number_input_string; start++)
                            {
                                order = true;
                                for (j = parameter_l, k = current_node->reference[start] + parameter_l; j < length_pattern[i]; j++, k++)
                                    if (pattern[i][j] != text[k])
                                    {
                                        order = false;
                                        break;
                                    }
                                if (order == true)
                                    printf("%d(%d),", k - 1, i + 1);
                            }
                        }

                        search = false;
                    }
                    else /* If the distance is not zero, the corresponding subtree will be searched */
                    {
                        start = current_node->children - 1;
                        order = false;
                        for (j = 0; j != start; j++)
                            if (current_node->distance[j] == HD)
                            {
                                current_node = current_node->child_node[j];
                                order = true;
                                break;
                            }
                        if (order == false)
                            search = false;
                    }
                }
                else /* If this node is leaf node, compare the string in this node with pattern one by one */
                {
                    number_input_string = current_node->number_reference;
                    for (start = 0; start != number_input_string; start++)
                    {
                        order = true;
                        for (j = 0, k = current_node->reference[start]; j < length_pattern[i]; j++, k++)
                            if (pattern[i][j] != text[k])
                            {
                                order = false;
                                break;
                            }
                        if (order == true)
                            printf("%d(%d),", k - 1, i + 1);
                    }

                    search = false;
                }
            }
        }
    }
    else /* Use 32-bit scheme to deal with the searching. Please consider the explanations of the 64-bit searching scheme. */
    {
        for (i = 0; i < number_pattern; i++)
        {
            for (j = 0; j < number_ull_for_a_string - 1; j++)
                for (k = 0; k < number_symbol_for_a_ull; k++)
                    bit_vector_for_pattern[j] = ((bit_vector_for_pattern[j] << number_bit_for_a_symbol) | alphabet[(int)pattern[i][j * number_symbol_for_a_ull + k]]) & filter_usless_bit_in_a_ull;
            for (k = 0; k < number_symbol_for_last_ull; k++)
                bit_vector_for_pattern[j] = ((bit_vector_for_pattern[j] << number_bit_for_a_symbol) | alphabet[(int)pattern[i][j * number_symbol_for_a_ull + k]]) & filter_usless_bit_in_last_ull;

            current_node = root;
            search = true;
            while (search == true)
            {
                if (current_node->children != 0)
                {
                    HD = 0;
                    for (j = 0; j < number_ull_for_a_string; j++)
                        HD += ComputeHammingDistance_32bits(((bit_vector_for_pattern[j] ^ current_node->bit_vector_reference[j]) + clear_bit_for_witness) & keep_bit_for_witness);

                    if (HD == 0)
                    {
                        number_input_string = current_node->number_reference;
                        if (parameter_l == length_pattern[i])
                        {
                            for (j = 0; j < number_input_string; j++)
                                printf("%d(%d),", current_node->reference[j] + parameter_l - 1, i + 1);
                        }
                        else
                        {
                            for (start = 0; start < number_input_string; start++)
                            {
                                order = true;
                                for (j = parameter_l, k = current_node->reference[start] + parameter_l; j < length_pattern[i]; j++, k++)
                                    if (pattern[i][j] != text[k])
                                    {
                                        order = false;
                                        break;
                                    }
                                if (order == true)
                                    printf("%d(%d),", k - 1, i + 1);
                            }
                        }

                        search = false;
                    }
                    else
                    {
                        start = current_node->children - 1;
                        order = false;
                        for (j = 0; j != start; j++)
                            if (current_node->distance[j] == HD)
                            {
                                current_node = current_node->child_node[j];
                                order = true;
                                break;
                            }
                        if (order == false)
                            search = false;
                    }
                }
                else
                {
                    number_input_string = current_node->number_reference;
                    for (start = 0; start != number_input_string; start++)
                    {
                        order = true;
                        for (j = 0, k = current_node->reference[start]; j < length_pattern[i]; j++, k++)
                            if (pattern[i][j] != text[k])
                            {
                                order = false;
                                break;
                            }
                        if (order == true)
                            printf("%d(%d),", k - 1, i + 1);
                    }

                    search = false;
                }
            }
        }
    }

    /* Set the time stamp of the end for searching */
    gettimeofday(&searching_end, NULL);

    /* Display the time for preprocessing phase, searching phase and total running */
    printf("\t%lf\t%lf\t%lf\n", (preprocessing_end.tv_sec - preprocessing_start.tv_sec) + (preprocessing_end.tv_usec - preprocessing_start.tv_usec)/1000000.0, (searching_end.tv_sec - searching_start.tv_sec) + (searching_end.tv_usec - searching_start.tv_usec)/1000000.0, (searching_end.tv_sec - preprocessing_start.tv_sec) + (searching_end.tv_usec - preprocessing_start.tv_usec)/1000000.0);

    /* Free the memories used in this program */
    free(text);
    for (i = 0; i < number_pattern; i++)
        free(pattern[i]);
    free(pattern);
    free(length_pattern);
    free(bit_vector_for_text);
    free(temp_bit_vector);
    free(input_string);
    free(sort_string);
    free(distance);
    free(counting);
    //free(bit_vector_for_pattern);
    FreeTree(&root);

    /* Return normal ending to system */
    return 0;
}

