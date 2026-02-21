# Defacto Rust Addon Makefile

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    ADDON_EXT = dylib
    LOADER_LDFLAGS = -ldl
else ifeq ($(UNAME_S),Linux)
    ADDON_EXT = so
    LOADER_LDFLAGS = -ldl
else
    # Windows
    ADDON_EXT = dll
    LOADER_LDFLAGS =
endif

LOADER = runtime_loader
ADDON = target/release/libsample_addon.$(ADDON_EXT)

.PHONY: all clean test release debug

all: release

release:
	@echo "🦀 Building Rust addon (release)..."
	cargo build --release
	@echo "✅ Built: $(ADDON)"

debug:
	@echo "🦀 Building Rust addon (debug)..."
	cargo build
	@echo "✅ Built: target/debug/libsample_addon.$(ADDON_EXT)"

test:
	@echo "🧪 Running Rust tests..."
	cargo test

clean:
	@echo "🧹 Cleaning..."
	cargo clean
	rm -f $(LOADER)

# Build the runtime loader (for testing addons)
$(LOADER): ../cpp/runtime_loader.cpp
	$(CXX) -std=c++17 -O2 -Wall -Wextra -o $@ $< $(LOADER_LDFLAGS)

loader: $(LOADER)

# Run test with sample addon
run: release $(LOADER)
	./$(LOADER) $(ADDON)
