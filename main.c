#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define ALPHABET_SIZE 26
#define MAX_TEXT_LENGTH 100000
#define TOP_N 3

// Function to read the distribution of letters from a file
void read_distribution(const char *filename, double distribution[ALPHABET_SIZE]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (fscanf(file, "%lf", &distribution[i]) != 1) {
            printf("Error reading distribution for letter %c\n", 'a' + i);
            exit(1);
        }
    }
    
    fclose(file);
}

// Function to compute the normalized frequency of each character
void compute_histogram(const char *text, double histogram[ALPHABET_SIZE]) {
    int count[ALPHABET_SIZE] = {0};
    int total_chars = 0;
    
    // Initialize histogram to zeros
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        histogram[i] = 0.0;
    }
    
    // Count occurrences of each letter
    for (int i = 0; text[i] != '\0'; i++) {
        if (isalpha(text[i])) {
            char c = tolower(text[i]);
            count[c - 'a']++;
            total_chars++;
        }
    }
    
    // Normalize to get frequencies
    if (total_chars > 0) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            histogram[i] = (double)count[i] / total_chars;
        }
    }
}

// Function to compute the Chi-square distance between two histograms
double chi_squared_distance(const double hist1[ALPHABET_SIZE], const double hist2[ALPHABET_SIZE]) {
    double distance = 0.0;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (hist2[i] > 0) {
            double diff = hist1[i] - hist2[i];
            distance += (diff * diff) / hist2[i];
        }
    }
    
    return distance;
}

// Function to compute the Euclidian distance between two histograms
double euclidean_distance(const double hist1[ALPHABET_SIZE], const double hist2[ALPHABET_SIZE]) {
    double sum = 0.0;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        double diff = hist1[i] - hist2[i];
        sum += diff * diff;
    }
    
    return sqrt(sum);
}

// Function to compute the Cosine distance between two histograms
double cosine_distance(const double hist1[ALPHABET_SIZE], const double hist2[ALPHABET_SIZE]) {
    double dot_product = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        dot_product += hist1[i] * hist2[i];
        norm1 += hist1[i] * hist1[i];
        norm2 += hist2[i] * hist2[i];
    }
    
    // Handle edge cases to avoid division by zero
    if (norm1 == 0.0 || norm2 == 0.0) {
        return 1.0; // Maximum distance
    }
    
    double similarity = dot_product / (sqrt(norm1) * sqrt(norm2));
    
    // Ensure similarity is in valid range [-1, 1]
    if (similarity > 1.0) similarity = 1.0;
    if (similarity < -1.0) similarity = -1.0;
    
    return 1.0 - similarity; // Convert similarity to distance
}

// Function to encrypt text with a specified shift
void encrypt_text(const char *input, char *output, int shift) {
    shift = shift % ALPHABET_SIZE;
    if (shift < 0) {
        shift += ALPHABET_SIZE;
    }
    
    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            char base = islower(input[i]) ? 'a' : 'A';
            output[i] = base + ((input[i] - base + shift) % ALPHABET_SIZE);
        } else {
            output[i] = input[i];
        }
    }
    output[strlen(input)] = '\0';
}

// Function to decrypt text with a specified shift
void decrypt_text(const char *input, char *output, int shift) {
    // Decryption is just encryption with the opposite shift
    shift = shift % ALPHABET_SIZE;
    if (shift < 0) {
        shift += ALPHABET_SIZE;
    }
    
    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            char base = islower(input[i]) ? 'a' : 'A';
            output[i] = base + ((input[i] - base - shift + ALPHABET_SIZE) % ALPHABET_SIZE);
        } else {
            output[i] = input[i];
        }
    }
    output[strlen(input)] = '\0';
}

// Function to break the Caesar cipher using frequency analysis
void break_caesar_cipher(const char* text, int top_shifts[TOP_N], double top_distances[TOP_N], 
                        double (*distance_function)(const double[], const double[])) {
    double english_dist[ALPHABET_SIZE];
    double text_hist[ALPHABET_SIZE];
    
    // Read standard English letter distribution
    read_distribution("distribution.txt", english_dist);
    
    // Compute histogram for the encrypted text
    compute_histogram(text, text_hist);
    
    // Initialize top shifts and distances
    for (int i = 0; i < TOP_N; i++) {
        top_shifts[i] = -1;
        top_distances[i] = 1e9; // A large value
    }
    
    // Try all possible shifts (0-25)
    for (int shift = 0; shift < ALPHABET_SIZE; shift++) {
        double shifted_english_dist[ALPHABET_SIZE];
        
        // Shift the English distribution to match the potential encryption shift
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            // If 'A' was encrypted to 'A'+shift, then the frequency of 'A'+shift in the
            // encrypted text should match the frequency of 'A' in English.
            int shifted_idx = (i + shift) % ALPHABET_SIZE;
            shifted_english_dist[shifted_idx] = english_dist[i];
        }
        
        // Calculate distance between shifted English distribution and encrypted text distribution
        double distance = distance_function(text_hist, shifted_english_dist);
        
        // Update top shifts if this distance is smaller
        for (int i = 0; i < TOP_N; i++) {
            if (distance < top_distances[i]) {
                // Shift all larger distances down
                for (int j = TOP_N - 1; j > i; j--) {
                    top_distances[j] = top_distances[j-1];
                    top_shifts[j] = top_shifts[j-1];
                }
                top_distances[i] = distance;
                top_shifts[i] = shift;
                break;
            }
        }
    }
}

// Function to read text from the keyboard
void read_text_from_keyboard(char *text) {
    printf("Enter text (max %d characters):\n", MAX_TEXT_LENGTH - 1);
    fgets(text, MAX_TEXT_LENGTH, stdin);
    text[strcspn(text, "\n")] = '\0'; // Remove trailing newline
}

// Function to read text from a file
int read_text_from_file(char *text, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return 0;
    }
    
    int chars_read = fread(text, sizeof(char), MAX_TEXT_LENGTH - 1, file);
    text[chars_read] = '\0';
    
    fclose(file);
    return 1;
}

// Function to display the letter frequency histogram
void display_histogram(const char *text) {
    double histogram[ALPHABET_SIZE];
    compute_histogram(text, histogram);
    
    printf("Letter Frequency Distribution:\n");
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        printf("%c: %.2f%%\n", 'a' + i, histogram[i] * 100);
    }
}

int main() {
    char text[MAX_TEXT_LENGTH] = "";
    char encrypted[MAX_TEXT_LENGTH] = "";
    char decrypted[MAX_TEXT_LENGTH] = "";
    char filename[100];
    int shift;
    int choice;
    
    do {
        printf("\n========== Caesar Cipher Menu ==========\n");
        printf("1. Read text from keyboard\n");
        printf("2. Read text from file\n");
        printf("3. Encrypt text with a specific shift\n");
        printf("4. Decrypt text with a known shift\n");
        printf("5. Display letter frequency distribution\n");
        printf("6. Break cipher using Chi-squared distance\n");
        printf("7. Break cipher using Euclidean distance\n");
        printf("8. Break cipher using Cosine distance\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline
        
        switch (choice) {
            case 1: // Read from keyboard
                read_text_from_keyboard(text);
                printf("Text read: %s\n", text);
                break;
                
            case 2: // Read from file
                printf("Enter filename: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0'; // Remove trailing newline
                
                if (read_text_from_file(text, filename)) {
                    printf("Text read from file:\n%s\n", text);
                }
                break;
                
            case 3: // Encrypt
                if (strlen(text) == 0) {
                    printf("Please read a text first.\n");
                    break;
                }
                
                printf("Enter shift value (0-25): ");
                scanf("%d", &shift);
                getchar(); // Consume newline
                
                encrypt_text(text, encrypted, shift);
                printf("Encrypted text: %s\n", encrypted);
                break;
                
            case 4: // Decrypt
                printf("Enter encrypted text: ");
                fgets(encrypted, MAX_TEXT_LENGTH, stdin);
                encrypted[strcspn(encrypted, "\n")] = '\0'; // Remove trailing newline
                
                printf("Enter shift value (0-25): ");
                scanf("%d", &shift);
                getchar(); // Consume newline
                
                decrypt_text(encrypted, decrypted, shift);
                printf("Decrypted text: %s\n", decrypted);
                break;
                
            case 5: // Display histogram
                if (strlen(text) == 0) {
                    printf("Please read a text first.\n");
                    break;
                }
                
                display_histogram(text);
                break;
                
            case 6: // Break cipher using Chi-squared
                printf("Enter encrypted text: ");
                fgets(encrypted, MAX_TEXT_LENGTH, stdin);
                encrypted[strcspn(encrypted, "\n")] = '\0'; // Remove trailing newline
                
                {
                    int top_shifts[TOP_N];
                    double top_distances[TOP_N];
                    
                    break_caesar_cipher(encrypted, top_shifts, top_distances, chi_squared_distance);
                    
                    printf("Top %d most likely encryption shifts using Chi-squared distance:\n", TOP_N);
                    for (int i = 0; i < TOP_N; i++) {
                        decrypt_text(encrypted, decrypted, top_shifts[i]);
                        printf("%d. Encryption Shift = %d, Distance = %.6f\n", i+1, top_shifts[i], top_distances[i]);
                        printf("   Decrypted: %s\n", decrypted);
                    }
                }
                break;
                
            case 7: // Break cipher using Euclidean
                printf("Enter encrypted text: ");
                fgets(encrypted, MAX_TEXT_LENGTH, stdin);
                encrypted[strcspn(encrypted, "\n")] = '\0'; // Remove trailing newline
                
                {
                    int top_shifts[TOP_N];
                    double top_distances[TOP_N];
                    
                    break_caesar_cipher(encrypted, top_shifts, top_distances, euclidean_distance);
                    
                    printf("Top %d most likely encryption shifts using Euclidean distance:\n", TOP_N);
                    for (int i = 0; i < TOP_N; i++) {
                        decrypt_text(encrypted, decrypted, top_shifts[i]);
                        printf("%d. Encryption Shift = %d, Distance = %.6f\n", i+1, top_shifts[i], top_distances[i]);
                        printf("   Decrypted: %s\n", decrypted);
                    }
                }
                break;
                
            case 8: // Break cipher using Cosine
                printf("Enter encrypted text: ");
                fgets(encrypted, MAX_TEXT_LENGTH, stdin);
                encrypted[strcspn(encrypted, "\n")] = '\0'; // Remove trailing newline
                
                {
                    int top_shifts[TOP_N];
                    double top_distances[TOP_N];
                    
                    break_caesar_cipher(encrypted, top_shifts, top_distances, cosine_distance);
                    
                    printf("Top %d most likely encryption shifts using Cosine distance:\n", TOP_N);
                    for (int i = 0; i < TOP_N; i++) {
                        decrypt_text(encrypted, decrypted, top_shifts[i]);
                        printf("%d. Encryption Shift = %d, Distance = %.6f\n", i+1, top_shifts[i], top_distances[i]);
                        printf("   Decrypted: %s\n", decrypted);
                    }
                }
                break;
                
            case 0: // Exit
                printf("Exiting program.\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);
    
    return 0;
}