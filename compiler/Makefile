CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = defacto
WIN_CXX ?= x86_64-w64-mingw32-g++
WIN_TARGET = defacto.exe
WIN_STRIP ?= x86_64-w64-mingw32-strip

# LLVM support (optional)
LLVM_CONFIG = llvm-config
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cflags 2>/dev/null)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags 2>/dev/null)
LLVM_LIBS = $(shell $(LLVM_CONFIG) --libs 2>/dev/null)
LLVM_SYSROOT = $(shell $(LLVM_CONFIG) --sysroot 2>/dev/null)

# Detect if LLVM is available
HAS_LLVM = $(shell command -v $(LLVM_CONFIG) >/dev/null 2>&1 && echo 1 || echo 0)

# Add LLVM flags if available
ifeq ($(HAS_LLVM), 1)
    CXXFLAGS += $(LLVM_CXXFLAGS)
    LDFLAGS += $(LLVM_LDFLAGS)
    LIBS += $(LLVM_LIBS)
    DEFINES += -DHAS_LLVM
endif

all: $(TARGET)

$(TARGET): main.cpp src/defacto.h src/lexer.h src/parser.h src/codegen.h src/llvm_codegen.h
	$(CXX) $(CXXFLAGS) $(DEFINES) -o $(TARGET) main.cpp $(LDFLAGS) $(LIBS)
	@echo "Built: $(TARGET)"
	@if [ $(HAS_LLVM) = 1 ]; then echo "  + LLVM backend enabled"; else echo "  - LLVM backend not available (install llvm-dev)"; fi

windows: main.cpp src/defacto.h src/lexer.h src/parser.h src/codegen.h
	$(WIN_CXX) $(CXXFLAGS) -static -o $(WIN_TARGET) main.cpp
	@$(WIN_STRIP) $(WIN_TARGET) 2>/dev/null || true
	@echo "built: $(WIN_TARGET)"

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/defacto
	@echo "installed: /usr/local/bin/defacto"

uninstall:
	rm -f /usr/local/bin/defacto
	@echo "uninstalled: /usr/local/bin/defacto"

brew:
	@echo "To install via Homebrew:"
	@echo "  brew tap vivooifo-droid/Defacto"
	@echo "  brew install defacto"

clean:
	rm -f $(TARGET) $(WIN_TARGET) *.o

# Help target
help:
	@echo "Defacto Compiler Build Targets:"
	@echo "  all       - Build defacto compiler (default)"
	@echo "  windows   - Build Windows executable"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  clean     - Remove built files"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Optional Features:"
	@echo "  LLVM backend - Install llvm-dev for optimized codegen"
