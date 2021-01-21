#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define true 1
#define false 0

typedef char bool;
typedef struct _LEAF
{
    int substring;
    struct _LEAF *next;
} leaf;

typedef struct _NODE
{
    int substring;
    int startPosition;
    int endPosition;
    struct _NODE **child;
    bool isLeaf;
    int numberOfLeaf;
    int *leafArray;
    struct _LEAF *storedLeaf;
    struct _LEAF *currentLeaf;
} node;

typedef struct _LEAFPOINTER
{
    node *validLeaf;
    struct _LEAFPOINTER *next;
} leafpointer;

void FreeCompactTreeLeaf (leaf *current)
{
   if (current->next != NULL)
   {
       FreeCompactTreeLeaf (current->next);
   }
   free(current);
}

void FreeCompactTree (node *current, const int sizeAlphabet)
{
    /* internal node */
    if (!current->isLeaf)
    {
        int i = 0;

        for (i = 0; i != sizeAlphabet; i++)
        {
            if (current->child[i] != NULL)
            {
                FreeCompactTree(current->child[i], sizeAlphabet);
            }
        }
        free(current->child);
    }
    /* leaf node */
    else
    {
        free(current->leafArray);
    }

    free(current);
}

int main (int argc, char **argv)
{
    FILE *textInput = NULL;
    FILE *patternInput = NULL;
    char *T = NULL;
    char **P = NULL;
    int n = 0;
    int r = 0;
    int *m = NULL;
    int sizeAlphabet = 0;
    unsigned char alphabet[256] = {};
    node *root = (node *)malloc(sizeof(node));
    node *current = NULL;
    node *parent = NULL;
    node *tempNode = NULL;
    int parameterL = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int s = 0;
    int d = 0;
    int e = 0;
    unsigned char tempChar = 0;
    bool continueFlag = false;
    bool mismatch = false;
    int matchedLength = 0;
    int tempMatchedLength = 0;
    bool needToSearch = false;
    leaf *templeaf = NULL;
    leaf *prepareForFreeLeaf = NULL;
    struct timeval preprocessingStart = {};
    struct timeval preprocessingEnd = {};
    struct timeval searchingStart = {};
    struct timeval searchingEnd = {};
    leafpointer *leafPointerRoot = NULL;
    leafpointer *leafPointerCurrent = NULL;
    leafpointer *tempLeafPointer = NULL;
    int *verifiedLeaf = NULL;
    
    if (argc != 4)
    {
        printf("This is a compact tire algorithm for solving the Multiple Exact Pattern Matching Problem.\n");
        printf("Usage: %s [text file] [pattern file (including multi-patterns)] [parameter l]\n", argv[0]);
        return 1;
    }

    if ((textInput = fopen(argv[1], "rb")) == NULL)
    {
        printf("Error! Cannot read the input text file (\"%s\")\n", argv[1]);
        return 2;
    }
    fseek(textInput, 0, SEEK_END);
    n = ftell(textInput);
    fseek(textInput, 0, SEEK_SET);
    T = (char *)malloc(n + 1);
    fread(T+1, 1, n, textInput);
    fclose(textInput);

    if ((patternInput = fopen(argv[2], "rb")) == NULL)
    {
        printf("Error! Cannot read the input pattern file (\"%s\")\n", argv[2]);
        free(T);
        return 3;
    }
    fscanf(patternInput, "%d\n", &r);
    P = (char **)malloc(sizeof(char *) * r);
    m = (int *)malloc(sizeof(int) * r);
    for (i = 0; i != r; i++)
    {
        fscanf(patternInput, "%d,", &m[i]);
        P[i] = (char *)malloc(m[i] + 1);
        fread(&P[i][1], 1, m[i], patternInput);
        fscanf(patternInput, "\n");
    }
    fclose(patternInput);

    parameterL = atoi(argv[3]);


    gettimeofday(&preprocessingStart, NULL);
    /* Scan text array once to obtain alphabet and size of alphabet */
    memset(alphabet, 0Xff, 256);
    for (i = 1; i <= n; i++)
    {
        if (alphabet[(unsigned char)T[i]] == 0xff)
        {
            alphabet[(unsigned char)T[i]] = 1;
            sizeAlphabet++;
        }
    }

    /* Give each symbol an unique number (lookup table) */
    j = 0;
    for (i = 0; i != 256; i++)
    {
        if (alphabet[i] != 0xff)
        {
            alphabet[i] = j;
            j++;
        }
    }

    /* Initialize root node which records the first substring */
    root->isLeaf = true;
    root->child = (node **)malloc(sizeAlphabet * sizeof(node *));
    /* We have to set all pointers to be NULL one by one, and we cannot use calloc or memset.
     * The reason is explained in "C Programming FAQs: Frequently Asked Questions" (Addison-Wesley, 1995, ISBN 0-201-84519-9)"
     * Or user can go to the web "http://c-faq.com/". 
     * In Questions 5.16 and 5.17, the issues have been discussed.
     * */
    for (i = 0; i != sizeAlphabet; i++)
    {
        root->child[i] = NULL;
    }
    root->isLeaf = true;

    leafPointerRoot = (leafpointer *)malloc(sizeof(leafpointer));
    leafPointerRoot->next = NULL;
    leafPointerCurrent = leafPointerRoot;

    /* Start construct compact tire by inserting substrings one by one */
    for (i = 1; i <= n - parameterL + 1; i++)
    {
        tempChar = (unsigned char)T[i];
        if (root->child[alphabet[tempChar]] != NULL)
        {
            current = root->child[alphabet[tempChar]];
        }
        else                                                                   /* no such child node and need to create it */
        {
            /* create a new child and store the triple */
            root->child[alphabet[tempChar]] = (node *)malloc(sizeof(node));
            if (root->isLeaf)
            {
                root->isLeaf = false;
            }
            current = root->child[alphabet[tempChar]];
            current->substring = i;
            current->startPosition = i;
            current->endPosition = i + parameterL - 1;
            /* This new node is a leaf */
            current->isLeaf = true;
            current->numberOfLeaf = 1;
            /* creat the linked list to store the substring which will be kept in this leaf */
            current->storedLeaf = (leaf *)malloc(sizeof(leaf));
            current->storedLeaf->substring = i;
            current->storedLeaf->next = NULL;
            current->currentLeaf = current->storedLeaf;

            leafPointerCurrent->next = (leafpointer *)malloc(sizeof(leafpointer));
            leafPointerCurrent = leafPointerCurrent->next;
            leafPointerCurrent->validLeaf = current;
            leafPointerCurrent->next = NULL;

            /* consider next substring */
            continue;
        }

        matchedLength = 0;
        continueFlag = true;
        parent = root;
        while (continueFlag)
        {
            /* Check whether the prefix of the substring stored in the current is equal to that of substring 
             * T[i ... i+paremeterL-1].
             * If yes, try to find the maximum length of command prefix.
             * If no, a new leaf node is needed.
             * */
            tempMatchedLength = 1;
            s = current->startPosition + 1;
            e = current->endPosition + 1;
            d = i + matchedLength + tempMatchedLength;
            mismatch = false;
            while (s < e)
            {
                if (T[s] != T[d])
                {
                    mismatch = true;
                    break;
                }
                s++;
                d++;
                tempMatchedLength++;
            }

            /* A mismatch occurs, and then we need to branch the current node and add a new leaf. */
            if (mismatch)
            {
                node *tempNode = NULL;

                tempChar = (unsigned char)T[current->startPosition];
                tempNode = parent->child[alphabet[tempChar]];
                /* update parent's link */
                parent->child[alphabet[tempChar]] = (node *)malloc(sizeof(node));
                current = parent->child[alphabet[tempChar]];
                /* update current branch */
                current->substring = tempNode->substring;
                current->startPosition = tempNode->startPosition;
                current->endPosition = s - 1;
                current->child = (node **)malloc(sizeAlphabet * sizeof(node *));
                for (j = 0; j != sizeAlphabet; j++)
                {
                     current->child[j] = NULL;
                }
                current->child[alphabet[(unsigned char)T[s]]] = tempNode;
                current->child[alphabet[(unsigned char)T[d]]] = (node *)malloc(sizeof(node));
                current->isLeaf = false;
                /* update the original node */
                tempNode->startPosition = s;
                /* update the new leaf node */
                current = current->child[alphabet[(unsigned char)T[d]]];
                current->substring = i;
                current->startPosition = d;
                current->endPosition = i + parameterL - 1;
                current->isLeaf = true;
                current->numberOfLeaf = 1;
                /* store the substring to this leaf node */
                current->storedLeaf = (leaf *)malloc(sizeof(leaf));
                current->storedLeaf->substring = i;
                current->storedLeaf->next = NULL;
                current->currentLeaf = current->storedLeaf;

                leafPointerCurrent->next = (leafpointer *)malloc(sizeof(leafpointer));
                leafPointerCurrent = leafPointerCurrent->next;
                leafPointerCurrent->validLeaf = current;
                leafPointerCurrent->next = NULL;

                continueFlag = false;
            }
            /* Continue to search the child node until a mismatch occurs or a leaf node is reached. */
            else
            {
                matchedLength += tempMatchedLength;
                /* There are three cases:
                 * 1. This node is a leaf node. Hence, the substring will be kept in this leaf node.
                 * 2. The substring is not long enough and then stop insert at this edge.
                 * 3. This node is an internal node. Therefore, we continue to search the child node.
                 *
                 * Note that Case 2 is impossible because every substrings are with the same lengths!
                 * */
                if (matchedLength == parameterL)                               /* Case 1 */
                {
                    /* store the substring into this leaf node */
                    (current->numberOfLeaf)++;
                    current->currentLeaf->next = (leaf *)malloc(sizeof(leaf));
                    current->currentLeaf = current->currentLeaf->next;
                    current->currentLeaf->substring = i;
                    current->currentLeaf->next = NULL;

                    continueFlag = false;
                }
                /* Case 2
                 * else if (current->startPosition + tempMatchedLegnth - 1 < current->endPosition)
                 * {
                 *
                 * }
                 * */
                else                                                           /* Case 3 */
                {
                    tempChar = (unsigned char)T[i + matchedLength];
                    if (current->child[alphabet[tempChar]] != NULL)
                    {
                        /* The corresponding child node is found, and then we continue to search the child node. */
                        /* Keep the information of parent node */
                        parent = current;

                        /* Go to child node */
                        current = current->child[alphabet[tempChar]];
                    }
                    /* No such child node exists, and then we create a new child node (leaf node). */
                    else
                    {
                        /* Creat a new leaf node */
                        current->child[alphabet[tempChar]] = (node *)malloc(sizeof(node));
                        current = current->child[alphabet[tempChar]];

                        /* Update this leaf node */
                        current->substring = i;
                        current->startPosition = i + matchedLength;
                        current->endPosition = i + parameterL - 1;
                        current->isLeaf = true;
                        current->numberOfLeaf = 1;
                        /* record the substring in this leaf node */
                        current->storedLeaf = (leaf *)malloc(sizeof(leaf));
                        current->storedLeaf->substring = i;
                        current->storedLeaf->next = NULL;
                        current->currentLeaf = current->storedLeaf;

                        leafPointerCurrent->next = (leafpointer *)malloc(sizeof(leafpointer));
                        leafPointerCurrent = leafPointerCurrent->next;
                        leafPointerCurrent->validLeaf = current;
                        leafPointerCurrent->next = NULL;

                        continueFlag = false;
                    }
                }
            }
        }
    }

    leafPointerCurrent = leafPointerRoot;
    while (leafPointerCurrent->next != NULL)
    {
        tempLeafPointer = leafPointerCurrent;
        leafPointerCurrent = leafPointerCurrent->next;
        free(tempLeafPointer);

        tempNode = leafPointerCurrent->validLeaf;
        j = tempNode->numberOfLeaf;
        tempNode->leafArray = (int *)malloc(sizeof(int) * j);
        templeaf = tempNode->storedLeaf;
        prepareForFreeLeaf = templeaf;
        for (i = 0; i != j; i++)
        {
            tempNode->leafArray[i] = templeaf->substring;
            templeaf = templeaf->next;
            free(prepareForFreeLeaf);
            prepareForFreeLeaf = templeaf;
        }
    }
    free (leafPointerCurrent);

    gettimeofday(&preprocessingEnd, NULL);

    gettimeofday(&searchingStart, NULL);
    /* Search patterns one by one from root node */
    for (i = 0; i != r; i++)
    {
        current = root;
        matchedLength = 0;
        needToSearch = true;
        do
        {
            /* Search reaches an internal node */
            if (!current->isLeaf)
            {
                tempChar = (unsigned char)P[i][matchedLength + 1];
                if (current->child[alphabet[tempChar]] != NULL)
                {
                    tempMatchedLength = 1;
                    /* Start the linear compare the substring stored in this node with the corresponding 
                     * substring of this pattern 
                     * */
                    s = current->child[alphabet[tempChar]]->startPosition + 1;
                    d = 1 + matchedLength + tempMatchedLength;
                    e = current->child[alphabet[tempChar]]->endPosition + 1;
                    mismatch = false;
                    while (s < e)
                    {
                        /* A mismatch occurs */
                        if (T[s] != d)
                        {
                            mismatch = true;
                            break;
                        }
                        s++;
                        d++;
                        tempMatchedLength++;
                    }
                    /* If there is a mismtach, stop search for current pattern */
                    if (mismatch)
                    {
                        needToSearch = false;
                    }
                    /* No mismatch occurs. Update the length of matchs and go to the child node */
                    else
                    {
                        matchedLength += tempMatchedLength;
                        current = current->child[alphabet[tempChar]];
                    }
                }
                else
                {
                    needToSearch = false;
                }
            }
            /* Search reaches a leaf node */
            else
            {
                /* Compare the substring stored in this leaf node with the corresponding substring of this pattern */
                verifiedLeaf = current->leafArray;
                k = current->numberOfLeaf;
                for (j = 0; j != k; j++)
                {
                    s = *(verifiedLeaf + j) + parameterL;
                    d = parameterL + 1;
                    e = m[i] + 1;
                    mismatch = false;
                    while (d < e)
                    {
                        if (T[s] != P[i][d])
                        {
                            mismatch = true;
                            break;
                        }
                        s++;
                        d++;
                    }
                    if (!mismatch)
                    {
                        if ((d == e) && (s <= n + 1))
                        {
                            printf("%d(%d),", s - 1, i + 1);
                        }
                    }
                }
                needToSearch = false;
            }
        } while (needToSearch);
    }
    gettimeofday(&searchingEnd, NULL);

    printf("\t%lf\t%lf\t%lf\n", (preprocessingEnd.tv_sec - preprocessingStart.tv_sec) + (preprocessingEnd.tv_usec - preprocessingStart.tv_usec)/1000000.0, (searchingEnd.tv_sec - searchingStart.tv_sec) + (searchingEnd.tv_usec - searchingStart.tv_usec)/1000000.0, (searchingEnd.tv_sec - preprocessingStart.tv_sec) + (searchingEnd.tv_usec - preprocessingStart.tv_usec)/1000000.0);
    free(T);
    FreeCompactTree(root, sizeAlphabet);
    for (i = 0; i != r; i++)
    {
        free(P[i]);
    }
    free(P);
    free(m);

    return 0;
}
