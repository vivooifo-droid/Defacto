#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXT_SRC="$SCRIPT_DIR/vscode-extension"

if [[ "$OSTYPE" == "darwin"* ]]; then
    EXT_DIR="$HOME/.vscode/extensions/defacto-lang"
elif [[ "$OSTYPE" == "linux"* ]]; then
    EXT_DIR="$HOME/.vscode/extensions/defacto-lang"
elif [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "cygwin"* ]]; then
    EXT_DIR="$USERPROFILE/.vscode/extensions/defacto-lang"
else
    echo "Unknown OS: $OSTYPE"
    echo "Manually copy 'vscode-extension/' to your VS Code extensions folder"
    exit 1
fi

echo "Installing Defacto syntax highlighting..."
echo "Source:      $EXT_SRC"
echo "Destination: $EXT_DIR"

rm -rf "$EXT_DIR"

cp -r "$EXT_SRC" "$EXT_DIR"

echo ""
echo "Done! Restart VS Code and open any .de file."
