# Defacto v0.52 Release Notes

**Release Date:** February 28, 2026
**Status:** Alpha

## Major Features

### Defo Package Manager

Complete rewrite of the package manager with full features:

```bash
# Initialize project
defo init my-app

# Install library
defo install owner/repo

# Build and run
defo build
defo run

# Manage dependencies
defo list
defo lock
defo info
```

**Features:**
- `defo.json` - Project configuration
- `defo.lock` - Dependency lock file
- Git-based installation
- Auto-detect platform
- Build integration

**defo.json format:**
```json
{
    "name": "my-app",
    "version": "0.1.0",
    "description": "My Defacto app",
    "author": "Your Name",
    "license": "MIT",
    "dependencies": {
        "stdlib": "vivooifo-droid/Defacto"
    }
}
```

---

### Standard Library

Complete standard library with working implementations:

#### Core Module (`core/`)
- Type aliases: `byte`, `word`, `dword`, `qword`, `ptr`
- Constants: `TRUE`, `FALSE`, `NULL`, numeric limits
- Functions: `nop()`, `panic()`, `assert()`

#### Math Module (`math/`)
- Basic: `abs()`, `min()`, `max()`, `clamp()`
- Power: `pow()`, `sqr()`, `cube()`
- Root: `sqrt()`
- Arithmetic: `mod()`, `div_round()`
- Bitwise: `is_bit_set()`, `set_bit()`, `clear_bit()`, `toggle_bit()`
- Sign: `is_positive()`, `is_negative()`, `sign()`

#### Collections Module (`collections/`)
- `Array` - Dynamic array
- `StringBuilder` - Mutable string buffer
- `Stack` - LIFO stack
- `Queue` - FIFO queue

#### I/O Module (`io/`)
- `println()`, `printch()`, `printi()`
- `readch()`, `readk()`
- `cls()`, `setcolor()` (bare-metal)

#### String Module (`string/`)
- `strlen()`, `strcpy()`, `strcmp()`
- `itoa()`

#### Test Module (`test/`)
- `assert_eq()`, `assert_true()`, `assert_false()`
- `test_print_results()`, `test_reset()`
- `run_all_tests()`

---

### Test Framework

Built-in unit testing framework:

```de
include "test.de"

fn test_math() {
    <.de
        var result: i32 = 0
        result = abs(-5)
        assert_eq(result, 5, "abs(-5) should be 5")
    .>
}

fn main {
    <.de
        run_all_tests()
    .>
}
```

**Output:**
```
Running tests...
====================
✓ abs(-5) should be 5
====================
Test Results:
  Run: 1
  Passed: 1
  Failed: 0
====================
All tests passed!
```

---

## Tools Updates

### defo.sh

Complete rewrite with:
- Color output
- Error handling
- Config file support
- Lock file generation
- Better dependency management

### deformat.sh

Code formatter for Defacto code.

### delint.sh

Code linter with checks for:
- Missing directives
- Unclosed sections
- Deprecated syntax
- Trailing whitespace

---

## Documentation

- Updated stdlib/README.md with complete API docs
- Added examples for all modules
- Updated main README with tools section

---

## Breaking Changes

None. All changes are backward compatible.

---

## Examples

### Using stdlib

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

include "core.de"
include "math.de"
include "io.de"

fn main {
    <.de
        var x: i32 = -5
        var y: i32 = 0
        
        y = abs(x)
        println(y)
        
        assert(y == 5, "abs should return positive")
    .>
}

#Mainprogramm.end
```

### Project Structure

```
my-app/
├── defo.json
├── defo.lock
├── src/
│   └── main.de
├── tests/
│   └── test_main.de
└── README.md
```

---

## System Requirements

- **macOS**: Xcode Command Line Tools, NASM
- **Linux**: Build essentials, NASM
- **ARM64**: macOS ARM or Linux ARM64

---

## Migration Guide

No migration needed. All existing code continues to work.

To use new stdlib:

```bash
# Install stdlib
defo install vivooifo-droid/Defacto

# Include in code
include "core.de"
include "math.de"
```

---

## Known Issues

- Collections module uses placeholders (needs proper allocator)
- String module needs full implementation
- Test framework needs test registration

---

## Contributors

Thanks to all contributors!

---

**Previous release:** [v0.51](RELEASE-v0.51.md)
