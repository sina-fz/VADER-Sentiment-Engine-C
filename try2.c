#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utility..h"
#include"math.h"

#define HASH_MAP_SIZE 100  // Size of the hash table
#define ARRAY_SIZE 10
#define MAX_STRING_LENGTH 50
#define MAX_LINE 256
#define MAX_TOKENS 50
#define MAX_TOKEN_SIZE 50

struct WordData* hash_map[HASH_MAP_SIZE];  // Hash map with separate chaining

// Hash function for strings
unsigned int hashFunction(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + *str;
        str++;
    }
    return hash % HASH_MAP_SIZE;
}

// Insert a word into the hash map
void insertWord(struct WordData *data) {
    unsigned int index = hashFunction(data->word);
    data->next = hash_map[index];
    hash_map[index] = data;
}

// Find a word in the hash map
struct WordData* findWord(const char *word) {
    unsigned int index = hashFunction(word);
    struct WordData *current = hash_map[index];
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;  // Word not found
}


// Function to read the lexicon file and store data in the hash map
void readFile() {
    FILE *reFi = fopen("vader_lexicon.txt", "r");
    if (reFi == NULL) {
        printf("Error in opening the file\n");
        return;
    }


    char line[MAX_LINE];
    while (fgets(line, sizeof(line), reFi) != NULL) {
        struct WordData *data = (struct WordData *)malloc(sizeof(struct WordData));
        if (data == NULL) {
            printf("Memory allocation failed!\n");
            fclose(reFi);
            return;
        }

        // Correct format string for sscanf
        int matches = sscanf(line, "%49s %f %f", data->word, &data->value1, &data->value2);
        if (matches == 3) {
            char *arrayStart = strchr(line, '[');
            if (arrayStart != NULL) {
                arrayStart++;
                char *arrayEnd = strchr(arrayStart, ']');
                if (arrayEnd != NULL) {
                    *arrayEnd = '\0';
                    char *token = strtok(arrayStart, ", ");
                    for (int j = 0; j < ARRAY_SIZE && token != NULL; j++) {
                        data->intArray[j] = atoi(token);
                        token = strtok(NULL, ", ");
                    }
                    data->next = NULL;  // Initialize next to NULL
                    insertWord(data);    // Insert the word into the hash map
                }
            }
        } else {
            printf("Error reading word or values\n");
            free(data);
        }
    }
    fclose(reFi);
}

// Function to tokenize a sentence
char** tokenize(char *sent) {
    const char *delimiters = ", .!?:;";
    char *start = sent;
    char *end;
    int i = 0;

    char **tokens = (char **)malloc(MAX_TOKENS * sizeof(char *));
    if (tokens == NULL) {
        printf("Memory allocation failed for tokens array.\n");
        return NULL;
    }

    while ((end = strpbrk(start, delimiters)) != NULL) {
        int token_length = end - start;
        tokens[i] = (char *)malloc((token_length + 1) * sizeof(char));
        if (tokens[i] == NULL) {
            for (int j = 0; j < i; j++) free(tokens[j]);
            free(tokens);
            return NULL;
        }
        strncpy(tokens[i], start, token_length);
        tokens[i][token_length] = '\0';
        start = end + 1;
        i++;
        if (i >= MAX_TOKENS) break;
    }

    if (*start != '\0') {
        tokens[i] = (char *)malloc((strlen(start) + 1) * sizeof(char));
        strcpy(tokens[i], start);
        i++;
    }
    tokens[i] = NULL;
    return tokens;
}

// Function to compute sentiment score using VADER rules, using the hash map for lookups
float sentimentVader(char **sentence) {
    float boost_Fac = 0.293;
    float reduce_Fac = -0.293;
    float negation_Fac = -0.5;
    float total_score = 0.0;

    char *P_amp[] = {"absolutely", "completely", "extremely", "really", "so", "totally", "very", "particularly", "exceptionally", "incredibly", "remarkably"};
    int num_P_amp = sizeof(P_amp) / sizeof(P_amp[0]);

    char *N_amp[] = {"barely", "hardly", "scarcely", "somewhat", "mildly", "slightly", "partially", "fairly", "pretty much"};
    int num_N_amp = sizeof(N_amp) / sizeof(N_amp[0]);

    char *negations[] = {"not", "isn't", "doesn't", "wasn't", "shouldn't", "won't", "cannot", "can't", "nor", "neither", "without", "lack", "missing"};
    int num_negations = sizeof(negations) / sizeof(negations[0]);

    for (int i = 0; sentence[i] != NULL; i++) {
        struct WordData *wordData = findWord(sentence[i]);
        if (wordData != NULL) {
            float word_score = wordData->value1;

            // Check for positive and negative amplifiers
            if (i > 0) {
                for (int k = 0; k < num_P_amp; k++) {
                    if (strcmp(sentence[i - 1], P_amp[k]) == 0) {
                        word_score += word_score * boost_Fac;
                        break;
                    }
                }
                for (int k = 0; k < num_N_amp; k++) {
                    if (strcmp(sentence[i - 1], N_amp[k]) == 0) {
                        word_score += word_score * reduce_Fac;
                        break;
                    }
                }
                // Check for negations
                for (int k = 0; k < num_negations; k++) {
                    if (strcmp(sentence[i - 1], negations[k]) == 0) {
                        word_score *= negation_Fac;
                        break;
                    }
                }
            }

            // Amplify score for ALLCAPS words
            if (isupper(sentence[i][0])) {
                word_score *= 1.5;
            }
            total_score += word_score;
             
        } else {
            printf("Word '%s' not found in lexicon.\n", sentence[i]);
        }
    }
    float com_score = total_score/sqrt(pow(total_score,2) +15);
    return com_score;
}

int main() {
    // Initialize hash map
    memset(hash_map, 0, sizeof(hash_map));

    // Read lexicon file and populate hash map
    readFile();

    char sentence1[] = "VADER is smart, handsome, and funny.";
    char sentence2[] = "VADER is smart, handsome, and funny!";
    char sentence3[] = "VADER is very smart, handsome, and funny.";
    char sentence4[] = "VADER is VERY SMART, handsome, and FUNNY.";
    char sentence5[] = "VADER is VERY SMART, handsome, and FUNNY!!!";
    char sentence6[] = "VADER is VERY SMART, handsome, and FUNNY!!!";
    char **tokens = tokenize(sentence1);
    
    float score = sentimentVader(tokens);
    printf("Sentiment Score: %.2f\n", score);

    // Free allocated memory for tokens
    for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
    free(tokens);
    return 0;
}
