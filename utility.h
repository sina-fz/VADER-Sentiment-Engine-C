#ifndef WORDDATA_H
#define WORDDATA_H

#define ARRAY_SIZE 10
#define MAX_STRING_LENGTH 50

// Define the WordData struct without a typedef
typedef struct  {
    char word[MAX_STRING_LENGTH];
    float value1; // Mean sentiment value
    float value2; // Standard deviation
    int intArray[ARRAY_SIZE]; // Array of sentiment ratings
    // struct WordData *next;  // Pointer for linked list in case of collision
}WordData;

#endif
