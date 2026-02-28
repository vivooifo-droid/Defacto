# Defacto v0.51 Release Notes

**Release Date:** February 28, 2026
**Status:** Alpha

## Major Features

### Bitwise Operators

Full support for bitwise operations in expressions:

```de
var a: i32 = 12
var b: i32 = 5

result = (a & b)   // AND: 4
result = (a | b)   // OR: 13
result = (a ^ b)   // XOR: 9
result = (a << 1)  // Left shift: 24
result = (a >> 1)  // Right shift: 6
```

**Supported operators:**
- `&` — AND
- `|` — OR
- `^` — XOR
- `<<` — Left shift
- `>>` — Right shift

### Package Manager (defo)

New package manager for Defacto libraries:

```bash
# Initialize project
./tools/defo.sh init my-app

# Install library
./tools/defo.sh install owner/repo

# List installed
./tools/defo.sh list

# Build
./tools/defo.sh build

# Run
./tools/defo.sh run
```

### Development Tools

**Code Formatter (`deformat.sh`):**
```bash
./tools/deformat.sh program.de
```

**Code Linter (`delint.sh`):**
```bash
./tools/delint.sh program.de
# Checks for:
# - Missing #Mainprogramm.start/end
# - Unclosed sections
# - Deprecated syntax
# - Trailing whitespace
```

### Examples

Complete feature examples in `examples/features.de`:
- Bitwise operators
- Compound assignment
- Type aliases
- Inline assembly
- Switch/case
- Arrays in structs
- Extern functions
- For loops
- Comparison operators
- ARM64 native code

## Compiler Changes

### ARM64 Codegen Updates

Added bitwise operators to ARM64 codegen:
- `AND` — Bitwise AND
- `ORR` — Bitwise OR
- `EOR` — Bitwise XOR
- `LSL` — Logical shift left
- `LSR` — Logical shift right

### x86 Codegen Updates

Added bitwise operators:
- `AND` — Bitwise AND
- `OR` — Bitwise OR
- `XOR` — Bitwise XOR
- `SHL` — Shift left
- `SHR` — Shift right

## Standard Library

Updated stdlib with working examples:
- `stdlib/core/core.de` — Type aliases
- `stdlib/math/math.de` — Using bitwise ops
- `stdlib/io/io.de` — I/O functions
- `stdlib/string/string.de` — String utilities

## Documentation

- Updated README.md with tools section
- Added examples/features.de
- Updated SYNTAX.md with bitwise operators

## Breaking Changes

None. All changes are backward compatible.

## Known Issues

- Inline assembly (`asm {}`) not yet supported in ARM64 codegen
- Full 64-bit register support for x86-64 still in progress
- Package manager search not implemented

## Migration Guide

No migration needed. All existing code continues to work.

## System Requirements

- **macOS**: Xcode Command Line Tools, NASM
- **Linux**: Build essentials, NASM
- **ARM64**: macOS ARM or Linux ARM64, NASM or GNU as

## Contributors

Thanks to all contributors!

---

**Previous release:** [v0.50](RELEASE-v0.50.md)
