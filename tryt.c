#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

#define ARRAY_SIZE 10
#define MAX_STRING_LENGTH 50
#define MAX_LINE 256

// Define WordData struct
typedef struct {
    char word[MAX_STRING_LENGTH];
    float value1;
    float value2;
    int intArray[ARRAY_SIZE];
} WordData;

// Function to count lines in the lexicon file
void countLines(FILE *file, int *line_count) {
    char line[MAX_LINE];
    *line_count = 0;
    while (fgets(line, sizeof(line), file)) {
        (*line_count)++;
    }
}

// Function to read the lexicon file
WordData* readLexiconFile(const char *filename, int *data_size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening lexicon file");
        return NULL;
    }

    int line_count = 0;
    countLines(file, &line_count);
    rewind(file);

    WordData *lexicon = malloc(line_count * sizeof(WordData));
    if (!lexicon) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    int i = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        int matches = sscanf(line, "%49[^\t]\t%f\t%f", lexicon[i].word, &lexicon[i].value1, &lexicon[i].value2);
        if (matches == 3) {
            char *array_start = strchr(line, '[');
            if (array_start) {
                array_start++;
                char *array_end = strchr(array_start, ']');
                if (array_end) {
                    *array_end = '\0';
                    char *token = strtok(array_start, ", ");
                    for (int j = 0; j < ARRAY_SIZE && token; j++) {
                        lexicon[i].intArray[j] = atoi(token);
                        token = strtok(NULL, ", ");
                    }
                }
            }
            i++;
        }
    }

    fclose(file);
    *data_size = i;
    return lexicon;
}

// Function to tokenize a sentence
WordData* tokenizeSentence(const char *sentence, int *token_count) {
    char delimiters[] = " ,.!?";
    char temp_sentence[MAX_LINE];
    strncpy(temp_sentence, sentence, MAX_LINE - 1);
    temp_sentence[MAX_LINE - 1] = '\0';

    char *token = strtok(temp_sentence, delimiters);
    int count = 0;
    WordData *tokens = malloc(sizeof(WordData) * MAX_LINE / 2); // Approximate allocation
    if (!tokens) {
        perror("Memory allocation failed");
        return NULL;
    }

    while (token) {
        strncpy(tokens[count].word, token, MAX_STRING_LENGTH - 1);
        tokens[count].word[MAX_STRING_LENGTH - 1] = '\0';
        tokens[count].value1 = 0;
        tokens[count].value2 = 0;
        memset(tokens[count].intArray, 0, sizeof(tokens[count].intArray));
        count++;
        token = strtok(NULL, delimiters);
    }

    *token_count = count;
    return tokens;
}

// Function to check amplifiers
bool isAmplifier(const char *word, float *effect) {
    char *amplifiers[] = {"very", "extremely", "absolutely", "completely", "totally"};
    for (int i = 0; i < sizeof(amplifiers) / sizeof(amplifiers[0]); i++) {
        if (strcmp(word, amplifiers[i]) == 0) {
            *effect = 0.293;
            return true;
        }
    }
    return false;
}

// Function to compute sentiment score
float computeSentiment(WordData *lexicon, int lexicon_size, WordData *tokens, int token_count) {
    float score = 0.0;
    float amplifier_effect = 0.0;
    for (int i = 0; i < token_count; i++) {
        bool found = false;
        for (int j = 0; j < lexicon_size; j++) {
            if (strcmp(tokens[i].word, lexicon[j].word) == 0) {
                score += lexicon[j].value1 * (1 + amplifier_effect);
                amplifier_effect = 0.0; // Reset after use
                found = true;
                break;
            }
        }

        // Check if the token is an amplifier
        if (!found) {
            isAmplifier(tokens[i].word, &amplifier_effect);
        }
    }

    return score / sqrt(score * score + 15);
}

// Main function
int main() {
    const char *filename = "vader_lexicon.txt";
    int lexicon_size;
    WordData *lexicon = readLexiconFile(filename, &lexicon_size);
    if (!lexicon) return EXIT_FAILURE;

    char sentence[] = "VADER is very smart, handsome, and funny!";
    int token_count;
    WordData *tokens = tokenizeSentence(sentence, &token_count);

    float sentiment_score = computeSentiment(lexicon, lexicon_size, tokens, token_count);
    printf("Compound Sentiment Score: %f\n", sentiment_score);

    free(lexicon);
    free(tokens);
    return EXIT_SUCCESS;
}
