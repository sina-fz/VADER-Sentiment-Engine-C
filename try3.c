#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include <ctype.h>
#include <math.h>

#define ARRAY_SIZE 10
#define MAX_STRING_LENGTH 50
#define Max_line 256
#define MAX_TOKENS 50
#define MAX_TOKEN_SIZE 50

// Count the number of lines in the file
void getLine(FILE *reFi, int *n){
    char line[Max_line];
    while (fgets(line, sizeof(line), reFi)) {
        (*n)++;
    }
}

WordData* readFile(int *data_size) {
    FILE *reFi = fopen("vader_lexicon.txt", "r");
    if (reFi == NULL) {
        printf("Error in opening the file\n");
        return NULL;
    }

    int n_line = 0;
    getLine(reFi, &n_line);
    rewind(reFi);

    WordData *data = (WordData *)malloc(n_line * sizeof(WordData));
    if (data == NULL) {
        printf("Memory allocation failed!\n");
        fclose(reFi);
        return NULL;
    }

    int i = 0;
    char line[Max_line];
    while (fgets(line, sizeof(line), reFi) != NULL) {
        int matches = sscanf(line, "%49[^\t]\t%f\t %f", data[i].word, &data[i].value1, &data[i].value2);
        if (matches == 3) {
            char *arrayStart = strchr(line, '[');
            if (arrayStart != NULL) {
                arrayStart++;
                char *arrayEnd = strchr(arrayStart, ']');
                if (arrayEnd != NULL) {
                    *arrayEnd = '\0';
                    char *token = strtok(arrayStart, ", ");
                    for (int j = 0; j < ARRAY_SIZE && token != NULL; j++) {
                        data[i].intArray[j] = atoi(token);
                        token = strtok(NULL, ", ");
                    }
                    i++;
                }
            }
        } else {
            printf("Error reading word or values at line %d\n", i + 1);
        }
    }
    fclose(reFi);
    *data_size = i;  // Set the actual number of entries read
    return data;
}



WordData* tokenize(char *sent) {
    const char *delimiters = " ,.";
    char *start = sent;
    int i = 0;

    WordData *tokens = (WordData *)malloc(MAX_TOKENS * sizeof(WordData));
    if (tokens == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    while (*start != '\0') {
        // Check if *start is a delimiter or special character
        if (strchr(delimiters, *start) != NULL || *start == '!' || *start == '?') {
            // Copy the word before the delimiter
            if (start != sent) {
                int token_length = start - sent;
                if (token_length < MAX_STRING_LENGTH) {
                    strncpy(tokens[i].word, sent, token_length);
                    tokens[i].word[token_length] = '\0';  // Null-terminate
                    tokens[i].value1 = 0;
                    i++;
                }
            }

            // Handle single-character punctuation as a separate token
            if (*start == '!' || *start == '?') {
                tokens[i].word[0] = *start;
                tokens[i].word[1] = '\0';
                tokens[i].value1 = 0;
                i++;
            }

            // Move past the delimiter
            start++;
            sent = start;
        } else {
            start++;
        }

        // Check for max tokens
        if (i >= MAX_TOKENS) {
            break;
        }
    }

    // Add the last token if there's any remaining text
    if (start != sent) {
        strncpy(tokens[i].word, sent, MAX_STRING_LENGTH - 1);
        tokens[i].word[MAX_STRING_LENGTH - 1] = '\0';
        tokens[i].value1 = 0;  // Ensure null-termination
        i++;
    }

    return tokens;
}


float sentimentVader(WordData* data, int data_size, WordData* tokens, int token_count) {
    float boost_Fac = 0.293;
    float reduce_Fac = -0.293;
    float negation_Fac = -0.5;
    float total_score = 0.0;
    int num_exc = 0;

    char *P_amp[] = {"absolutely", "completely", "extremely", "really", "so", "totally", "very", "particularly", "exceptionally", "incredibly", "remarkably"};
    int num_P_amp = sizeof(P_amp) / sizeof(P_amp[0]);

    char *N_amp[] = {"barely", "hardly", "scarcely", "somewhat", "mildly", "slightly", "partially", "fairly", "pretty much"};
    int num_N_amp = sizeof(N_amp) / sizeof(N_amp[0]);

    char *negations[] = {"not", "isn't", "doesn't", "wasn't", "shouldn't", "won't", "cannot", "can't", "nor", "neither", "without", "lack", "missing"};
    int num_negations = sizeof(negations) / sizeof(negations[0]);

    for (int i = 0; i < token_count; i++) {
        float All_cap = 1.0;
        int proc = 0;
        //int word_size = sizeof(tokens[i].word)/sizeof(tokens[i].word[0]);
        //for (int t = 0; tokens[i])

        if (isupper(tokens[i].word[0])) {
            All_cap = 1.5;
            for (char *p = tokens[i].word; *p; ++p) *p = tolower(*p);
        }

        for (int j = 0; j < data_size; j++) {
            if (strcmp(data[j].word, tokens[i].word) == 0) {
                tokens[i].value1  = data[j].value1;



                for (int p = 0; p < num_P_amp; p++) {
                    if (tokens[i-1].word == P_amp[p]){
                        tokens[i-1].value1 =1;
                    }
                    if (i > 0 && strcmp(tokens[i - 1].word, P_amp[p]) == 0) {
                        total_score += (data[j].value1 * All_cap) * (1 + boost_Fac);
                        proc = 1;
                        break;
                    }
                }

                for (int n = 0; n < num_N_amp; n++) {
                    if (tokens[i-1].word == N_amp[n]){
                        tokens[i-1].value1 =1;
                    }                    
                    if (i > 0 && strcmp(tokens[i - 1].word, N_amp[n]) == 0) {
                        total_score += (data[j].value1 * All_cap) * (1 - reduce_Fac);
                        proc = 1;
                        break;
                    }
                }

                for (int e = 0; e < num_negations; e++) {
                    if (tokens[i-1].word == negations[e]){
                        tokens[i-1].value1 =1;
                    }   
                    if (i > 0 && strcmp(tokens[i - 1].word, negations[e]) == 0) {
                        total_score += data[j].value1 * All_cap * negation_Fac;
                        proc = 1;
                        break;
                    }
                }
                if(tokens[i-1].value1 == 0 && i >1){
                strcpy(tokens[i-1].word, tokens[i-2].word);
                tokens[i-1].value1 = tokens[i-2].value1;
                }

                if (proc == 0) {
                    total_score += data[j].value1 * All_cap;
                }
                break;
            }
        }

        if (strcmp(tokens[i].word, "!") == 0 && num_exc < 3) {
            total_score += (total_score > 0 ? 0.292 : -0.292);
            num_exc++;
        } else if (strcmp(tokens[i].word, "!") != 0) {
            num_exc = 0;
        }
    }

    float comp_score = total_score / (sqrt((total_score * total_score) + 15));
    return comp_score;
}

int main() {
    int data_size;
    WordData* data = readFile(&data_size);  // Load the lexicon data
    if (data == NULL) {
        return 1;
    }

    char sentence[] = " smart funny ";
    WordData *tokens = tokenize(sentence);  // Tokenize the sentence
    if (tokens == NULL) {
        free(data);
        return 1;
    }

    // Count the number of tokens
    int token_count = 0;
    while (tokens[token_count].word[0] != '\0') {
        token_count++;
    }
    for (int i = 0; i<token_count; i++){
    }
    printf ("%d",token_count);

    // Calculate the sentiment score
    float score = sentimentVader(data, data_size, tokens, token_count);
    printf("The compound score is %f\n", score);

    // Free allocated memory
    free(data);
    free(tokens);

    return 0;
}
