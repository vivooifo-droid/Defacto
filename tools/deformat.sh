#!/bin/bash
# Defacto Code Formatter
# Usage: deformat <file.de>

if [ -z "$1" ]; then
    echo "Usage: deformat <file.de>"
    exit 1
fi

FILE="$1"
if [ ! -f "$FILE" ]; then
    echo "File not found: $FILE"
    exit 1
fi

# Simple formatter using sed
cat "$FILE" | \
    sed 's/[[:space:]]*$//' | \
    sed 's/^{/ {/' | \
    sed 's/^}/}/' | \
    sed 's/  */ /g' | \
    sed '/^$/N;/^\n$/d' | \
    sed 's/\t/    /g' > "$FILE.tmp"

mv "$FILE.tmp" "$FILE"
echo "Formatted: $FILE"
