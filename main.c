#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utility.h"

// Array of sentences to analyze
const char *sentences[] = {
    "VADER is smart, handsome, and funny.",
    "VADER is smart, handsome, and funny!",
    "VADER is very smart, handsome, and funny.",
    "VADER is VERY SMART, handsome, and FUNNY.",
    "VADER is VERY SMART, handsome, and FUNNY!!!",
    "VADER is VERY SMART, uber handsome, and FRIGGIN FUNNY!!!",
    "VADER is not smart, handsome, nor funny.",
    "At least it isn't a horrible book",
    "The plot was good, but the characters are uncompelling and the dialogue is not great.",
    "Make sure you :) or :D today!",
    "Not bad at all"
};

// Function to demonstrate the basic sentiment analysis process
int main() {
    Node *head = NULL;  // The head of the linked list (dictionary)
    
    // Load the sentiment lexicon (assumes the file is "vader_lexicon.txt")
    int result = loadDict("vader_lexicon.txt", &head);
    if (result != 0) {
        printf("Error loading dictionary.\n");
        return 1;
    }

    // Loop through each sentence and calculate its sentiment score
    for (int i = 0; i < sizeof(sentences) / sizeof(sentences[0]); i++) {
        const char *sentence = sentences[i];
        float sentiment_score = calculateSentiment(sentence, head);
        printf("Sentiment score for sentence '%s': %f\n", sentence, sentiment_score);
    }

    // Free the dictionary (linked list) memory
    freeDict(head);

    return 0;
}

