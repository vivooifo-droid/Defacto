#!/bin/bash

set -e
cd "$(dirname "$0")/compiler"

if [[ "$1" == "--win" ]]; then
    make clean
    if ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
        echo "error: x86_64-w64-mingw32-g++ not found"
        echo "Install MinGW-w64 cross-compiler and retry."
        exit 1
    fi
    make windows
    echo ""
    echo "Windows compiler ready: compiler/defacto.exe"
    exit 0
fi

make clean
make
echo ""
echo "Compiler ready: compiler/defacto"
echo ""
echo "Usage:"
echo "  ./compiler/defacto -terminal hello.de"
echo "  ./compiler/defacto -kernel -o kernel.bin os.de"
echo "  ./compiler/defacto -h"
