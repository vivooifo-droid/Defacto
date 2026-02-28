#!/bin/bash
# Defacto Package Manager (defo)
# Usage: defo <command> [args]

DEFACTO_DIR="${DEFACTO_DIR:-$HOME/.defacto}"
LIBS_DIR="$DEFACTO_DIR/libs"

mkdir -p "$LIBS_DIR"

case "$1" in
    init)
        if [ -z "$2" ]; then
            echo "Usage: defo init <project-name>"
            exit 1
        fi
        mkdir -p "$2"
        cat > "$2/main.de" << 'DEOF'
#Mainprogramm.start
#NO_RUNTIME
#SAFE

fn main {
    <.de
        var msg: string = "Hello from Defacto!"
        display{msg}
    .>
}

#Mainprogramm.end
DEOF
        echo "Initialized Defacto project: $2"
        ;;
    
    install)
        if [ -z "$2" ]; then
            echo "Usage: defo install <library>"
            exit 1
        fi
        echo "Installing $2..."
        # Simple git clone for now
        if [[ "$2" == *"://"* ]]; then
            git clone "$2" "$LIBS_DIR/$(basename $2 .git)"
        else
            git clone "https://github.com/$2.git" "$LIBS_DIR/$(basename $2)"
        fi
        echo "Installed $2"
        ;;
    
    list)
        echo "Installed libraries:"
        ls -1 "$LIBS_DIR" 2>/dev/null || echo "  (none)"
        ;;
    
    search)
        if [ -z "$2" ]; then
            echo "Usage: defo search <query>"
            exit 1
        fi
        echo "Searching for: $2"
        echo "Note: Search not implemented yet. Browse: https://github.com/topics/defacto"
        ;;
    
    build)
        echo "Building..."
        if [ -f "main.de" ]; then
            ../compiler/defacto main.de -o main
        elif [ -f "$1.de" ]; then
            ../compiler/defacto "$1.de" -o "$1"
        else
            echo "No main.de or specified file found"
            exit 1
        fi
        ;;
    
    run)
        $0 build
        if [ -f "./main" ]; then
            ./main
        elif [ -f "./$1" ]; then
            ./"$1"
        fi
        ;;
    
    clean)
        rm -f *.o *.asm main a.out
        echo "Cleaned"
        ;;
    
    *)
        echo "Defacto Package Manager v0.51"
        echo ""
        echo "Usage: defo <command> [args]"
        echo ""
        echo "Commands:"
        echo "  init <name>     Initialize new project"
        echo "  install <lib>   Install library (git URL or owner/repo)"
        echo "  list            List installed libraries"
        echo "  search <query>  Search for libraries"
        echo "  build [file]    Build project"
        echo "  run [file]      Build and run"
        echo "  clean           Clean build artifacts"
        ;;
esac
