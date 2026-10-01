# Caesar Cipher Encryptor, Decryptor & Analyzer

[![CI](https://github.com/Bogzx/Caesar-Cypher-Decryptor/actions/workflows/ci.yml/badge.svg)](https://github.com/Bogzx/Caesar-Cypher-Decryptor/actions/workflows/ci.yml)
[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A command-line tool written in C to perform Caesar cipher encryption, decryption, and frequency analysis-based cryptanalysis. This program allows users to work with text from keyboard input or files, and employs statistical methods to attempt to break unknown Caesar ciphers.

## 🌟 Features

* **Text Input:**
  * Read text directly from keyboard input.
  * Load text from a specified file.
* **Caesar Cipher Operations:**
  * **Encrypt:** Encrypt the loaded text with a user-specified shift (0-25).
  * **Decrypt:** Decrypt text (either currently loaded/encrypted or newly provided) with a known shift.
* **Frequency Analysis:**
  * Display the letter frequency distribution (histogram) of the loaded text.
* **Cipher Breaking (Cryptanalysis):**
  * Attempt to break a Caesar-encrypted text using frequency analysis.
  * Utilizes a standard English letter frequency distribution (`distribution.txt`).
  * Employs three different statistical distance/similarity metrics:
    1. **Chi-squared Distance**
    2. **Euclidean Distance**
    3. **Cosine Distance**
  * Suggests the Top N (default 3) most likely original encryption shifts and shows the corresponding decrypted text.
  * Can use previously loaded/encrypted text or new input for breaking.
* **User-Friendly Menu:** Interactive command-line menu for easy operation.
* **`distribution.txt` Handling:**
  * Reads expected letter frequencies from `distribution.txt`.
  * If `distribution.txt` is not found, it attempts to create a dummy version with standard English letter frequencies.

## 🛠️ Getting Started

### Prerequisites

* A C compiler (e.g., GCC, Clang).
* The `math.h` library (usually standard, but linking might be needed).

### Compilation

Clone the repository and build with `make` (or call the compiler directly):

```bash
git clone https://github.com/Bogzx/Caesar-Cypher-Decryptor.git
cd Caesar-Cypher-Decryptor
make                                  # or: gcc -std=c99 -O2 main.c -o caesar -lm
```

`-lm` links the math library, needed for `sqrt` in the distance calculations.

### Running the Program

Run the executable from the repository directory, so it finds `distribution.txt`:

```bash
./caesar
```

You will be greeted with the main menu. Enter `0` or press `Ctrl+D` (end of input) to exit.

Breaking a ciphertext with option 6 (the shift is unknown; the program ranks all 26):

```
Enter your choice: 6
Enter encrypted text: Aol xbpjr iyvdu mve qbtwz vcly aol shgf kvn
Top 3 most likely encryption shifts using Chi-squared distance:
1. Encryption Shift = 7, Distance = 2.527562
   Decrypted: The quick brown fox jumps over the lazy dog
2. Encryption Shift = 13, Distance = 2.912688
   Decrypted: Nby kocwe vliqh zir dogjm ipyl nby futs xia
3. Encryption Shift = 19, Distance = 3.089862
   Decrypted: Hvs eiwqy pfckb tcl xiadg cjsf hvs zonm rcu
```

### Running the Tests

```bash
make test
```

This drives the menu through stdin and checks encryption, decryption, all three cipher-breaking metrics and input handling. Each case has a 5-second limit; sanitizer builds can start slowly, so CI runs them with `make test TEST_TIMEOUT=30`.

## 📖 How to Use

The program operates via a numerical menu. Enter the number corresponding to your desired action.

### `distribution.txt` File

* This file is crucial for the cipher-breaking functionality (options 6, 7, 8).
* It should contain 26 floating-point numbers, each on a new line, representing the expected frequencies of the letters 'a' through 'z' in the target language (e.g., English).
* Example format for `distribution.txt`:
  ```
  0.08167
  0.01492
  ...
  0.00074
  ```
  (the first line is the frequency of 'a', the last of 'z'; comments are not allowed).
* If this file is not present when a cipher is broken, the program uses built-in approximate English letter frequencies and writes them to `distribution.txt`.

### Menu Options

1. **Read text from keyboard:**

   * Prompts you to enter text directly into the console.
   * This text is stored as the "loaded text".
   * The previously encrypted text is cleared.
2. **Read text from file:**

   * Prompts you for a filename.
   * Reads the content of the specified file into the "loaded text" (up to 99,999 characters; a longer file is truncated with a warning).
   * The previously encrypted text is cleared.
3. **Encrypt loaded text with a specific shift:**

   * Requires text to be loaded first (via option 1 or 2).
   * Prompts for an encryption shift value (0-25).
   * The "loaded text" is encrypted, and the result is stored as the "encrypted text".
4. **Decrypt text with a known shift:**

   * Prompts for the ciphertext. Press Enter on an empty line to use the last "encrypted text", or, if there is none, the "loaded text".
   * Prompts for the decryption shift value (0-25).
   * Displays the decrypted text.
5. **Display letter frequency distribution of loaded text:**

   * Requires text to be loaded first (via option 1 or 2).
   * Calculates and displays the percentage frequency of each letter (a-z) in the current "loaded text".
6. **Break cipher using Chi-squared distance:**
7. **Break cipher using Euclidean distance:**
8. **Break cipher using Cosine distance:**

   * These options attempt to automatically decrypt a Caesar cipher.
   * **Input Text Priority:** you are prompted for the ciphertext. On an empty line:
     1. If "encrypted text" exists (from option 3 or a previous decrypt/break), it is used.
     2. Else, the "loaded text" (from option 1 or 2) is used.
   * The program compares the letter frequencies of the input text (shifted by all 26 possibilities) against the standard frequencies from `distribution.txt` using the chosen distance metric.
   * It then prints the Top N (default 3) most probable encryption shifts and the corresponding decrypted plaintexts. A smaller distance indicates a higher likelihood.
0. **Exit:**

   * Terminates the program.

Only the ASCII letters `a-z`/`A-Z` are shifted and counted; digits, punctuation and non-ASCII characters (e.g. `é`, `ă`) are left unchanged.

### Workflow Examples

* **Encrypting a file:**

  1. Choose option `2` to load text from `my_secret_message.txt`.
  2. Choose option `3`, enter desired shift (e.g., `7`).
  3. The console will display the encrypted text. You can copy this or modify the program to save it.
* **Breaking a cipher from a file:**

  1. Ensure `distribution.txt` is present or allow the program to create it.
  2. Choose option `2` to load your ciphertext file (e.g., `unknown_cipher.txt`).
  3. Choose option `6`, `7`, or `8` and press Enter on the empty prompt to use the loaded text.
  4. Review the suggested decryptions.
* **Breaking a cipher pasted in:**

  1. Choose option `6` (or `7`, `8`).
  2. When prompted, paste your ciphertext.
  3. Review suggestions.

## 📊 Frequency Analysis & Distance Metrics

The core of the cipher-breaking functionality relies on the principle that different letters appear with different characteristic frequencies in a given language.
A Caesar cipher merely shifts these frequencies. By comparing the frequency distribution of an encrypted text (after trying all 26 possible reverse shifts) to the known standard letter frequency distribution of a language (e.g., English), we can identify the most probable original shift.

The "distance" or "similarity" between two frequency distributions (vectors of 26 numbers) is measured by:

* **Chi-squared Distance ($ \chi^2 $):** A statistical test that measures how well the observed frequencies (from the potentially decrypted text) fit the expected frequencies (from `distribution.txt`). Lower values indicate a better fit.
* **Euclidean Distance:** The straight-line distance between the two frequency vectors in 26-dimensional space. Lower values mean the distributions are more similar.
* **Cosine Distance:** Calculates the cosine of the angle between the two frequency vectors. Cosine similarity is 1 if vectors are identical, 0 if orthogonal. Cosine distance is `1 - Cosine Similarity`. Lower distance values indicate more similarity.

Enjoy using the Caesar Cipher tool!
