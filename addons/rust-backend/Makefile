# Defacto Backend Framework Makefile

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    ADDON_EXT = dylib
else ifeq ($(UNAME_S),Linux)
    ADDON_EXT = so
else
    ADDON_EXT = dll
endif

ADDON = target/release/libdefacto_backend.$(ADDON_EXT)

.PHONY: all clean test release debug

all: release

release:
	@echo "🦀 Building Defacto Backend (release)..."
	cargo build --release
	@echo "✅ Built: $(ADDON)"

debug:
	@echo "🦀 Building Defacto Backend (debug)..."
	cargo build
	@echo "✅ Built: target/debug/libdefacto_backend.$(ADDON_EXT)"

test:
	@echo "🧪 Running tests..."
	cargo test

clean:
	@echo "🧹 Cleaning..."
	cargo clean
