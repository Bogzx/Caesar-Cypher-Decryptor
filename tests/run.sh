#!/bin/sh
# End-to-end tests: drive the interactive menu through stdin and check the output.
# Usage: tests/run.sh path/to/caesar
set -u

BIN=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cp "$ROOT/distribution.txt" "$WORK/"
cd "$WORK" || exit 1

PLAIN="The quick brown fox jumps over the lazy dog while the old farmer watches from the barn and eats his breakfast"
failures=0

# Guard against infinite loops where `timeout` exists (not on stock macOS)
if command -v timeout >/dev/null 2>&1; then TIMEOUT="timeout 5"; else TIMEOUT=""; fi

# run NAME INPUT EXPECTED — pipe INPUT into the program and grep its output for EXPECTED
run() {
    out=$(printf '%b' "$2" | $TIMEOUT "$BIN" 2>&1)
    code=$?
    if [ $code -ne 0 ]; then
        echo "FAIL $1: exit code $code"
        failures=$((failures + 1))
    elif ! printf '%s' "$out" | grep -qF -- "$3"; then
        echo "FAIL $1: expected output to contain: $3"
        printf '%s\n' "$out" | tail -n 15
        failures=$((failures + 1))
    else
        echo "ok   $1"
    fi
}

run "encrypt" "1\n$PLAIN\n3\n7\n0\n" "Encrypted text: Aol xbpjr iyvdu mve"
run "decrypt" "4\nAol xbpjr iyvdu mve\n7\n0\n" "Decrypted text: The quick brown fox"
run "decrypt reuses last ciphertext" "1\n$PLAIN\n3\n7\n4\n\n7\n0\n" "Decrypted text: The quick brown fox"
run "break chi-squared" "1\n$PLAIN\n3\n7\n6\n\n0\n" "1. Encryption Shift = 7,"
run "break euclidean" "1\n$PLAIN\n3\n7\n7\n\n0\n" "1. Encryption Shift = 7,"
run "break cosine" "1\n$PLAIN\n3\n7\n8\n\n0\n" "1. Encryption Shift = 7,"
printf 'Aol xbpjr iyvdu mve qbtwz vcly aol shgf kvn dopsl aol vsk mhytly dhajolz' > cipher.txt
run "break text loaded from file" "2\ncipher.txt\n6\n\n0\n" "1. Encryption Shift = 7,"
run "loading new text clears last ciphertext" "1\nabc\n3\n1\n1\nxyz\n4\n\n0\n0\n" "Decrypted text: xyz"
run "histogram" "1\naab\n5\n0\n" "a: 66.67%"
run "non-ASCII passes through" "1\nCafé Ăla\n3\n1\n0\n" "Encrypted text: Dbgé Ămb"
run "invalid choice" "abc\n0\n" "Invalid choice"
run "invalid shift" "1\nabc\n3\n99\n0\n" "Invalid shift"
run "EOF exits instead of looping" "1\nabc\n" "End of input"
run "EOF mid-prompt exits" "4\n" "End of input"

rm -f distribution.txt
run "missing distribution.txt falls back to defaults" "6\nAol xbpjr iyvdu mve qbtwz vcly aol shgf kvn\n0\n" "1. Encryption Shift = 7,"
if [ -f distribution.txt ]; then echo "ok   default distribution.txt written"; else echo "FAIL default distribution.txt not written"; failures=$((failures + 1)); fi

printf 'not a number\n' > distribution.txt
run "malformed distribution.txt reports error" "6\nabc\n0\n" "Error reading distribution"

if [ $failures -ne 0 ]; then
    echo "$failures test(s) failed"
    exit 1
fi
echo "all tests passed"
