#!/bin/bash
# Defacto Code Linter
# Usage: delint <file.de>

if [ -z "$1" ]; then
    echo "Usage: delint <file.de>"
    exit 1
fi

FILE="$1"
ERRORS=0
WARNINGS=0

echo "Linting: $FILE"
echo "---"

# Check for #Mainprogramm.start
if ! grep -q "#Mainprogramm.start" "$FILE"; then
    echo "❌ ERROR: Missing #Mainprogramm.start"
    ((ERRORS++))
fi

# Check for #Mainprogramm.end
if ! grep -q "#Mainprogramm.end" "$FILE"; then
    echo "❌ ERROR: Missing #Mainprogramm.end"
    ((ERRORS++))
fi

# Check for unclosed sections
OPEN=$(grep -c "<.de" "$FILE" || true)
CLOSE=$(grep -c ".>" "$FILE" || true)
if [ "$OPEN" != "$CLOSE" ]; then
    echo "❌ ERROR: Unclosed section (<.de ... .>)"
    ((ERRORS++))
fi

# Check for static.pl> (deprecated in v0.44+)
if grep -q "static.pl>" "$FILE"; then
    echo "⚠️  WARNING: static.pl> is deprecated, consider removing"
    ((WARNINGS++))
fi

# Check for old function syntax
if grep -q "function ==" "$FILE"; then
    echo "⚠️  WARNING: 'function ==' is deprecated, use 'fn' instead"
    ((WARNINGS++))
fi

# Check for old for loop syntax
if grep -qE "for.*;.*;" "$FILE"; then
    echo "⚠️  WARNING: Old for loop syntax detected, use 'for i = 0 to 10'"
    ((WARNINGS++))
fi

# Check for old driver syntax
if grep -q "<drv." "$FILE"; then
    echo "⚠️  WARNING: Old driver syntax detected, use 'driver name { type = ... }'"
    ((WARNINGS++))
fi

# Check for trailing whitespace
if grep -q "[[:space:]]$" "$FILE"; then
    echo "⚠️  WARNING: Trailing whitespace detected"
    ((WARNINGS++))
fi

echo "---"
echo "Errors: $ERRORS, Warnings: $WARNINGS"

if [ $ERRORS -gt 0 ]; then
    exit 1
fi
exit 0
