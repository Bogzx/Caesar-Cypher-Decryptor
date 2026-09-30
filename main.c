#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define ALPHABET_SIZE 26
#define MAX_TEXT_LENGTH 100000
#define TOP_N 3

// Approximate English letter frequencies (a-z), used when distribution.txt is missing
static const double DEFAULT_ENGLISH_DISTRIBUTION[ALPHABET_SIZE] = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074
};

// Function to write the default English distribution to a file
void write_default_distribution(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        fprintf(file, "%.5f\n", DEFAULT_ENGLISH_DISTRIBUTION[i]);
    }

    fclose(file);
}

// Function to read the distribution of letters from a file.
// Falls back to (and writes out) the default English distribution if the file is missing.
// Returns 1 on success, 0 if the file exists but is malformed.
int read_distribution(const char *filename, double distribution[ALPHABET_SIZE]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("%s not found, using default English letter frequencies (written to %s).\n",
               filename, filename);
        memcpy(distribution, DEFAULT_ENGLISH_DISTRIBUTION, sizeof(DEFAULT_ENGLISH_DISTRIBUTION));
        write_default_distribution(filename);
        return 1;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (fscanf(file, "%lf", &distribution[i]) != 1) {
            printf("Error reading distribution for letter %c in %s\n", 'a' + i, filename);
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}

// Function to read one line from stdin, without the trailing newline.
// Returns 0 on end of input.
int read_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return 0;
    }

    size_t len = strcspn(buffer, "\n");
    if (buffer[len] == '\n') {
        buffer[len] = '\0';
    } else {
        // Line was longer than the buffer: discard the rest of it
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
    return 1;
}

// Function to read an integer from its own line of stdin.
// Returns 1 on success, 0 on invalid input, -1 on end of input.
int read_int(int *value) {
    char line[64];
    if (!read_line(line, sizeof(line))) {
        return -1;
    }

    char *end;
    long parsed = strtol(line, &end, 10);
    while (isspace((unsigned char)*end)) {
        end++;
    }
    if (end == line || *end != '\0') {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

// Only ASCII letters are counted and shifted; other bytes (digits, punctuation,
// UTF-8 multibyte characters) pass through unchanged.
int is_ascii_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
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
        if (is_ascii_letter(text[i])) {
            char c = (char)tolower((unsigned char)text[i]);
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
        if (is_ascii_letter(input[i])) {
            char base = (input[i] >= 'a' && input[i] <= 'z') ? 'a' : 'A';
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
        if (is_ascii_letter(input[i])) {
            char base = (input[i] >= 'a' && input[i] <= 'z') ? 'a' : 'A';
            output[i] = base + ((input[i] - base - shift + ALPHABET_SIZE) % ALPHABET_SIZE);
        } else {
            output[i] = input[i];
        }
    }
    output[strlen(input)] = '\0';
}

// Function to break the Caesar cipher using frequency analysis.
// Returns 0 if the reference distribution could not be loaded.
int break_caesar_cipher(const char* text, int top_shifts[TOP_N], double top_distances[TOP_N],
                        double (*distance_function)(const double[], const double[])) {
    double english_dist[ALPHABET_SIZE];
    double text_hist[ALPHABET_SIZE];
    
    // Read standard English letter distribution
    if (!read_distribution("distribution.txt", english_dist)) {
        return 0;
    }
    
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
    return 1;
}

// Function to read text from the keyboard
int read_text_from_keyboard(char *text) {
    printf("Enter text (max %d characters):\n", MAX_TEXT_LENGTH - 1);
    return read_line(text, MAX_TEXT_LENGTH);
}

// Function to read text from a file
int read_text_from_file(char *text, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return 0;
    }
    
    size_t chars_read = fread(text, sizeof(char), MAX_TEXT_LENGTH - 1, file);
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

// Function to read a ciphertext. On an empty line it falls back to the last
// encrypted text, or else to the loaded text.
// Returns 1 if there is text to work on, 0 if not, -1 on end of input.
int read_encrypted_text(char *encrypted, const char *loaded) {
    static char line[MAX_TEXT_LENGTH];

    if (strlen(encrypted) > 0) {
        printf("Enter encrypted text (empty line = use last encrypted text): ");
    } else if (strlen(loaded) > 0) {
        printf("Enter encrypted text (empty line = use loaded text): ");
    } else {
        printf("Enter encrypted text: ");
    }
    if (!read_line(line, sizeof(line))) {
        return -1;
    }

    if (strlen(line) > 0) {
        strcpy(encrypted, line);
    } else if (strlen(encrypted) == 0) {
        strcpy(encrypted, loaded);
    }
    if (strlen(encrypted) == 0) {
        printf("No encrypted text given.\n");
        return 0;
    }
    return 1;
}

// Function to read a shift value in the range 0-25.
// Returns 1 on success, 0 on invalid input, -1 on end of input.
int read_shift(int *shift) {
    printf("Enter shift value (0-25): ");
    int status = read_int(shift);
    if (status == 1 && (*shift < 0 || *shift >= ALPHABET_SIZE)) {
        status = 0;
    }
    if (status == 0) {
        printf("Invalid shift. Please enter a number between 0 and 25.\n");
    }
    return status;
}

// Function to break a ciphertext with the given metric and print the top candidates
void run_cipher_break(const char *encrypted, const char *metric_name,
                      double (*distance_function)(const double[], const double[])) {
    static char decrypted[MAX_TEXT_LENGTH];
    int top_shifts[TOP_N];
    double top_distances[TOP_N];

    if (!break_caesar_cipher(encrypted, top_shifts, top_distances, distance_function)) {
        return;
    }

    printf("Top %d most likely encryption shifts using %s distance:\n", TOP_N, metric_name);
    for (int i = 0; i < TOP_N; i++) {
        decrypt_text(encrypted, decrypted, top_shifts[i]);
        printf("%d. Encryption Shift = %d, Distance = %.6f\n", i+1, top_shifts[i], top_distances[i]);
        printf("   Decrypted: %s\n", decrypted);
    }
}

int main(void) {
    static char text[MAX_TEXT_LENGTH] = "";
    static char encrypted[MAX_TEXT_LENGTH] = "";
    static char decrypted[MAX_TEXT_LENGTH] = "";
    char filename[256];
    int shift;
    int choice;
    int status;
    
    for (;;) {
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

        status = read_int(&choice);
        if (status == -1) {
            printf("\nEnd of input. Exiting program.\n");
            return 0;
        }
        if (status == 0) {
            printf("Invalid choice. Please try again.\n");
            continue;
        }
        
        switch (choice) {
            case 1: // Read from keyboard
                if (read_text_from_keyboard(text)) {
                    encrypted[0] = '\0';
                    printf("Text read: %s\n", text);
                }
                break;
                
            case 2: // Read from file
                printf("Enter filename: ");
                if (read_line(filename, sizeof(filename)) &&
                    read_text_from_file(text, filename)) {
                    encrypted[0] = '\0';
                    printf("Text read from file:\n%s\n", text);
                }
                break;
                
            case 3: // Encrypt
                if (strlen(text) == 0) {
                    printf("Please read a text first.\n");
                    break;
                }
                
                if (read_shift(&shift) != 1) {
                    break;
                }
                
                encrypt_text(text, encrypted, shift);
                printf("Encrypted text: %s\n", encrypted);
                break;
                
            case 4: // Decrypt
                if (read_encrypted_text(encrypted, text) != 1 || read_shift(&shift) != 1) {
                    break;
                }
                
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
                if (read_encrypted_text(encrypted, text) == 1) {
                    run_cipher_break(encrypted, "Chi-squared", chi_squared_distance);
                }
                break;
                
            case 7: // Break cipher using Euclidean
                if (read_encrypted_text(encrypted, text) == 1) {
                    run_cipher_break(encrypted, "Euclidean", euclidean_distance);
                }
                break;
                
            case 8: // Break cipher using Cosine
                if (read_encrypted_text(encrypted, text) == 1) {
                    run_cipher_break(encrypted, "Cosine", cosine_distance);
                }
                break;
                
            case 0: // Exit
                printf("Exiting program.\n");
                return 0;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}
