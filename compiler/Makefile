CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = defacto
WIN_CXX ?= x86_64-w64-mingw32-g++
WIN_TARGET = defacto.exe
WIN_STRIP ?= x86_64-w64-mingw32-strip

all: $(TARGET)

$(TARGET): main.cpp src/defacto.h src/lexer.h src/parser.h src/codegen.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

windows: main.cpp src/defacto.h src/lexer.h src/parser.h src/codegen.h
	$(WIN_CXX) $(CXXFLAGS) -static -o $(WIN_TARGET) main.cpp
	@$(WIN_STRIP) $(WIN_TARGET) 2>/dev/null || true
	@echo "built: $(WIN_TARGET)"

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/defacto
	@echo "installed: /usr/local/bin/defacto"

clean:
	rm -f $(TARGET) $(WIN_TARGET) *.o
