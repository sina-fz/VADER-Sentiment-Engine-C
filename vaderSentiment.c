#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

#define ARRAY_SIZE 10
#define MAX_STRING_LENGTH 50
#define Max_line 256
#define MAX_TOKEN_SIZE 50

// Count the number of lines in the file
void getLine(FILE *reFi, int *n){
    char line[Max_line];
    while (fgets(line, sizeof(line), reFi)) {
        (*n)++;
    }
}

int countWords(char *sentence) {
    int count = 0;
    char str_copy[100];
    strcpy(str_copy, sentence);

    // Iterate through each character of the sentence until we hit the null terminator
    while (*sentence != '\0') {
        if (*sentence == '!') {  // Check if the current character matches 'ch'
            count++;  // Increment the count for each occurrence
        }
        sentence++;  // Move to the next character
    }

    char *token = strtok(str_copy, " ,!?");
     // Tokenize by space, comma, period, etc.

    // Tokenize and count words
    while (token != NULL) {
        count++;  // Increment count for each word
        token = strtok(NULL, " ,.?!");  // Continue tokenizing
    }
    
    return count;
}
bool isAllCaps(const char* word) {
    for (const char* p = word; *p != '\0'; ++p) {
        if (!isupper(*p) && !isspace(*p)) { // Ensure only uppercase letters are allowed
            return false;
        }
    }
    return true;
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

WordData* tokenize(char *sent, int *token_count) {
    const char *delimiters = " ,.?";
    char *start = sent;
    int i = 0;

    WordData *tokens = (WordData *)malloc((*token_count) * sizeof(WordData));
    if (tokens == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    while (*start != '\0' && i < *token_count) {
        if (strchr(delimiters, *start) != NULL || *start == '!') {
            if (start != sent) {
                int token_length = start - sent;
                if (token_length < MAX_STRING_LENGTH) {
                    strncpy(tokens[i].word, sent, token_length);
                    tokens[i].word[token_length] = '\0';
                    tokens[i].value1 = 0;
                    tokens[i].value2 = 0;
                    memset(tokens[i].intArray, 0, sizeof(tokens[i].intArray));
                    i++;
                }
            }
            if (*start == '!') {
                tokens[i].word[0] = *start;
                tokens[i].word[1] = '\0';
                tokens[i].value1 = 0;
                tokens[i].value2 = 0;
                memset(tokens[i].intArray, 0, sizeof(tokens[i].intArray));
                i++;
            }
            start++;
            sent = start;
        } else {
            start++;
        }
    }

    if (start != sent && i < *token_count) {
        strncpy(tokens[i].word, sent, MAX_STRING_LENGTH - 1);
        tokens[i].word[MAX_STRING_LENGTH - 1] = '\0';
        tokens[i].value1 = 0;
        tokens[i].value2 = 0;
        memset(tokens[i].intArray, 0, sizeof(tokens[i].intArray));
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

    char *P_amp[] = {"absolutely", "completely", "extremely", "really", "so", "totally", "very", "particularly", "exceptionally", "incredibly", "remarkably","uber","friggin"};
    int num_P_amp = sizeof(P_amp) / sizeof(P_amp[0]);

    char *N_amp[] = {"barely", "hardly", "scarcely", "somewhat", "mildly", "slightly", "partially", "fairly", "pretty much"};
    int num_N_amp = sizeof(N_amp) / sizeof(N_amp[0]);

    char *negations[] = {"not", "isn't", "doesn't", "wasn't", "shouldn't", "won't", "cannot", "can't", "nor", "neither", "without", "lack", "missing"};
    int num_negations = sizeof(negations) / sizeof(negations[0]);
    // Array of words that stop negation
    char *negation_stoppers[] = {"but", "however", "although", "though", "yet", "still", "nevertheless", "on the other hand", "except", "in contrast", "conversely", "instead", "otherwise"};
    int num_stoppers = sizeof(negation_stoppers) / sizeof(negation_stoppers[0]);
    
    float amplifying_effect = 0.0;
    float negating_effect = 1.0;
    
    

    for (int i = 0; i < token_count; i++) {
        float All_cap = 1.0;
        
        


        // Check if the current word is all caps and convert to lowercase
        if (isAllCaps(tokens[i].word)) {
            for (char *p = tokens[i].word; *p != '\0'; ++p) {
                *p = tolower(*p);
            }
        }
        if (isupper(tokens[i].word[0])){
            for (char *p = tokens[i].word; *p != '\0'; ++p) {
                *p = tolower(*p);
            }
        }

        // Check if the current word is an amplifier or negation
        bool is_effect_word;
        if (i < token_count - 1) { // Ensure there's a next word
            for (int p = 0; p < num_P_amp; p++) {
                if (strcmp(tokens[i].word, P_amp[p]) == 0) {
                    amplifying_effect = boost_Fac * All_cap;
                    is_effect_word = true;
                    break;
                }
            }
            for (int n = 0; n < num_N_amp; n++) {
                if (strcmp(tokens[i].word, N_amp[n]) == 0) {
                    amplifying_effect = reduce_Fac * All_cap;
                    is_effect_word = true;
                    break;
                }
            }
            for (int s= 0; s<num_stoppers;s++){
                if(strcmp(tokens[i].word,negation_stoppers[s]) == 0){
                    negating_effect = 1;
                }
            }
            for (int e = 0; e < num_negations; e++) {
                if (strcmp(tokens[i].word, negations[e]) == 0) {
                    negating_effect = negation_Fac * All_cap;
                    is_effect_word = true;
                    break;
                }
            }

        }

        // If the word is an effect word, skip its impact unless the next word is in the dictionary
        if (is_effect_word) {
            i++; // Move to the next word
            if (i >= token_count) break; // Prevent out-of-bounds access

            // Reset All_cap for the next word
            All_cap = 1.0;
            if (isAllCaps(tokens[i].word)) {
                All_cap = 1.5;
                for (char *p = tokens[i].word; *p != '\0'; ++p) {
                    *p = tolower(*p);
                }
            }

            
        }
        // Check if current word matches dictionary
        int effectApplied = 0; // Flag to check if effects have been applied

        for (int j = 0; j < data_size; j++) {
            if (strcmp(data[j].word, tokens[i].word) == 0) {
                // Apply the effects if the word is in the dictionary
                printf("%f",negating_effect);
                float score_adjusted = data[j].value1 * All_cap * (1 + amplifying_effect) * negating_effect;
    
                tokens[i].value1 = data[j].value1;
                total_score += score_adjusted;
                
                // Mark that the effects have been applied
                effectApplied = 1;
                break;
            }
        }

        // Process exclamation marks
        if (strcmp(tokens[i].word, "!") == 0 && num_exc < 3) {
            total_score += (total_score > 0 ? 0.292 : -0.292);
            num_exc++;
        } else if (strcmp(tokens[i].word, "!") != 0) {
            num_exc = 0;
        }

        // If the current word isn't in the dictionary (effect was not applied), retain the effects for the next word
        if (!effectApplied) {
            // The effects will be carried over to the next word
            amplifying_effect = amplifying_effect;  // No reset yet, carry it over
            negating_effect = negating_effect;      // No reset yet, carry it over
            //printf("%f",negating_effect);
        }else {
            // if (negation_Fac != 1){
            //     int negating_effect= negating_effect;
            // }
            // negating_effect = 0.0;
            amplifying_effect = 0.0;
        }

    }
    float comp_score = total_score / (sqrt((total_score * total_score) + 15));
    return comp_score;

}



// Assume necessary headers and function prototypes (like readFile, countWords, tokenize, sentimentVader) are included.

int main() {
    int data_size;
    WordData* data = readFile(&data_size);  // Load the lexicon data
    if (data == NULL) {
        return 1;
    }

    // Define all the sentences from the table
    const char* sentences[] = {
        "VADER is smart, handsome, and funny.",
        "VADER is smart, handsome, and funny!",
        "VADER is very smart, handsome, and funny.",
        "VADER is VERY SMART, handsome, and FUNNY.",
        "VADER is VERY SMART, handsome, and FUNNY!!!",
        "VADER is VERY SMART, uber handsome, and FRIGGIN FUNNY!!!",
        "VADER is not smart, handsome, nor funny.",
        "At least it isn't a horrible book.",
        "The plot was good, but the characters are uncompelling and the dialog is not great.",
        "Make sure you :) or :D today!",
        "Not bad at all"
    };

    // Number of sentences
    int num_sentences = sizeof(sentences) / sizeof(sentences[0]);

    // Loop through each sentence and analyze sentiment
    for (int i = 0; i < num_sentences; i++) {
        const char* sentence = sentences[i];

        // Cast const char* to char* when passing to the functions
        int token_count = countWords((char*)sentence);
        WordData* tokens = tokenize((char*)sentence, &token_count);
        if (tokens == NULL) {
            free(data);
            return 1;
        }

        float score = sentimentVader(data, data_size, tokens, token_count);
        printf("Sentence: \"%s\"\n", sentence);
        printf("The compound score is %f\n\n", score);

        // Free memory allocated for tokens
        free(tokens);
    }

    // Free allocated memory for lexicon data
    free(data);

    return 0;
}

