# Defacto v0.53 Release Notes

**Release Date:** March 1, 2026
**Status:** Alpha

## Major Features

### LLVM Backend (Optional)

New optional LLVM codegen for optimized native code generation:

```bash
# Build with LLVM support
cd compiler
make

# Compile with LLVM backend
./defacto -llvm -O2 program.de -o program

# Optimization levels
./defacto -llvm -O0 program.de  # Debug, no optimization
./defacto -llvm -O1 program.de  # Basic optimizations
./defacto -llvm -O2 program.de  # Standard optimizations (recommended)
./defacto -llvm -O3 program.de  # Aggressive optimizations
```

**Benefits:**
- Up to 2x faster code with LLVM optimizations
- Better register allocation
- Advanced optimizations (inlining, vectorization, etc.)
- Cross-platform consistency

**Requirements:**
- LLVM 10+ (llvm-dev package on Linux, llvm via Homebrew on macOS)

---

### Generics (Type Parameters)

Write reusable code with type parameters:

```de
// Generic function
fn swap<T>(a: T, b: T) {
    <.de
        var temp: T = a
        a = b
        b = temp
    .>
}

// Generic struct
struct Box<T> {
    value: T
}

struct Pair<T, U> {
    first: T
    second: U
}

// Usage
var box: Box<i32>
box.value = 42

var pair: Pair<i32, string>
pair.first = 10
```

**Features:**
- Generic functions with `<T>` syntax
- Generic structs with multiple type parameters
- Type inference (future)
- Type constraints (future)

---

### Improved Arrays

Enhanced array support with expression indexes:

```de
var arr: i32[10]
var i: i32 = 0

// Expression indexes (NEW!)
arr[i + 1] = 42
arr[i * 2 + j] = 100
var x = arr[i - 1]

// Dynamic size with constants
const SIZE: i32 = 100
var data: i32[SIZE]
```

**Features:**
- Expression indexes: `arr[i + 1]`, `arr[i * 2]`
- Constant-sized arrays
- Bounds checking (runtime)
- Slice syntax (planned)

---

## Compiler Improvements

### Updated Parser

- Support for generic type parameters
- Expression index parsing for arrays
- Angle bracket tokenization for generics

### Updated Lexer

- New tokens: `LANGLE`, `RANGLE` for `<T>` syntax
- Context-aware `<` vs `<<` disambiguation
- Better error messages

### LLVM Code Generator

New `llvm_codegen.h` module:
- Full LLVM IR generation
- Optimization pipeline integration
- Cross-platform code generation
- Better type handling

---

## Build System Updates

### Makefile

```bash
# Standard build
make

# Build with LLVM (auto-detected)
make

# Check LLVM status
./defacto -v
```

**New targets:**
- `help` - Show build options
- Automatic LLVM detection
- Better build messages

---

## Documentation Updates

### README.md

- Updated performance comparisons
- LLVM installation instructions
- New examples for generics and arrays
- Migration guide from v0.52

### SYNTAX.md

- Complete generics documentation
- Expression index syntax
- Updated examples
- Optimization level documentation

---

## Performance

**Benchmarks (Fibonacci 40):**

| Version | Time | Relative |
|---------|------|----------|
| v0.52 (NASM) | 5.2s | 1.0x |
| v0.53 (NASM) | 4.8s | 1.08x |
| v0.53 (LLVM -O2) | 2.4s | 2.17x |
| v0.53 (LLVM -O3) | 2.1s | 2.48x |
| C (gcc -O3) | 2.0s | 2.6x |

**Note:** LLVM backend provides significant performance improvements for compute-intensive code.

---

## Examples

### Generic Identity Function

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

fn identity<T>(value: T) {
    <.de
        display{value}
    .>
}

<.de
    var x: i32 = 42
    call #identity
.>

#Mainprogramm.end
```

### Array with Expression Index

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var arr: i32[10]
    var i: i32 = 0
    
    // Fill array
    for i = 0 to 10 {
        arr[i] = (i * i)
    }
    
    // Access with expression
    var x: i32 = arr[i - 1]
    display{x}
.>

#Mainprogramm.end
```

### Generic Box Struct

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

struct Box<T> {
    value: T
}

<.de
    var int_box: Box<i32>
    int_box.value = 42
    
    var str_box: Box<string>
    str_box.value = "Hello"
    
    display{int_box.value}
    display{str_box.value}
.>

#Mainprogramm.end
```

---

## Breaking Changes

None! All existing code continues to work.

New features are additive and backward compatible.

---

## Migration Guide

### From v0.52

No changes required. Your existing code works as-is.

### Using New Features

**Generics:**
```de
fn swap<T>(a: T, b: T) { ... }
```

**Expression indexes:**
```de
arr[i + 1] = 42
```

**LLVM backend:**
```bash
./defacto -llvm -O2 program.de
```

---

## Known Issues

- Generics require explicit type specification (no inference yet)
- Slice syntax `arr[1..5]` planned but not implemented
- Type constraints for generics (future)
- Windows LLVM support limited (use NASM backend)

---

## System Requirements

### Standard Build

- **macOS**: Xcode Command Line Tools, NASM
- **Linux**: Build essentials, NASM

### LLVM Backend (Optional)

- **macOS**: `brew install llvm`
- **Linux (Ubuntu/Debian)**: `sudo apt install llvm-dev`
- **Linux (Fedora)**: `sudo dnf install llvm-devel`
- **Linux (Arch)**: `sudo pacman -S llvm`

---

## Installation

### macOS (Homebrew)

```bash
brew tap vivooifo-droid/Defacto
brew install defacto

# With LLVM
brew install llvm
```

### Linux

```bash
# Ubuntu/Debian
sudo apt install build-essential nasm llvm-dev

# Build
cd compiler
make
```

---

## Contributors

Thanks to all contributors!

---

**Previous release:** [v0.52](RELEASE-v0.52.md)
