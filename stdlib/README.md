# Defacto Standard Library

Complete standard library for Defacto programming language.

## Modules

### Core (`core/`)

Basic types, constants, and utilities.

```de
include "core.de"

fn main {
    <.de
        var x: byte = 42
        var ptr: ptr = NULL
        
        if x > 0 {
            display{"positive"}
        }
        
        assert(x == 42, "x should be 42")
    .>
}
```

**Types:**
- `byte` = u8
- `sbyte` = i8
- `word` = i32
- `dword` = i32
- `qword` = i64
- `ptr` = pointer

**Constants:**
- `TRUE`, `FALSE` - Boolean values
- `NULL` - Null pointer
- `I8_MIN`, `I8_MAX`, `U8_MAX` - Numeric limits
- `BUFFER_SMALL` (64), `BUFFER_MEDIUM` (256), `BUFFER_LARGE` (1024)
- `EXIT_SUCCESS` (0), `EXIT_FAILURE` (1)

**Functions:**
- `nop()` - No operation
- `panic(msg)` - Panic with message
- `assert(cond, msg)` - Assert condition

---

### Math (`math/`)

Mathematical functions.

```de
include "math.de"

fn main {
    <.de
        var x: i32 = -5
        var y: i32 = 0
        
        y = abs(x)      // y = 5
        y = min(3, 7)   // y = 3
        y = max(3, 7)   // y = 7
        y = pow(2, 3)   // y = 8
        y = sqr(4)      // y = 16
        y = sqrt(16)    // y = 4
    .>
}
```

**Functions:**

| Function | Description | Example |
|----------|-------------|---------|
| `abs(x)` | Absolute value | `abs(-5) = 5` |
| `min(a, b)` | Minimum | `min(3, 7) = 3` |
| `max(a, b)` | Maximum | `max(3, 7) = 7` |
| `clamp(v, min, max)` | Clamp value | `clamp(10, 0, 5) = 5` |
| `pow(base, exp)` | Power | `pow(2, 3) = 8` |
| `sqr(x)` | Square | `sqr(4) = 16` |
| `cube(x)` | Cube | `cube(3) = 27` |
| `sqrt(x)` | Square root | `sqrt(16) = 4` |
| `mod(a, b)` | Modulo | `mod(10, 3) = 1` |
| `is_bit_set(v, b)` | Check bit | `is_bit_set(5, 0) = true` |
| `set_bit(v, b)` | Set bit | `set_bit(0, 1) = 2` |
| `clear_bit(v, b)` | Clear bit | `clear_bit(3, 0) = 2` |
| `is_positive(x)` | Check positive | `is_positive(5) = true` |
| `is_negative(x)` | Check negative | `is_negative(-5) = true` |
| `sign(x)` | Sign function | `sign(-5) = -1` |

---

### Collections (`collections/`)

Data structures: Array, StringBuilder, Stack, Queue.

```de
include "collections.de"

fn main {
    <.de
        var arr: *Array = array_new(10)
        var stack: *Stack = stack_new(10)
        
        array_push(arr, 42)
        stack_push(stack, 100)
    .>
}
```

**Types:**
- `Array` - Dynamic array
- `StringBuilder` - Mutable string buffer
- `Stack` - LIFO stack
- `Queue` - FIFO queue

---

### I/O (`io/`)

Input/Output functions.

```de
include "io.de"

fn main {
    <.de
        println("Hello, World!")
        printi(42)
        var ch: i32 = readch()
    .>
}
```

**Functions:**
- `println(msg)` - Print string with newline
- `printch(ch)` - Print character
- `printi(num)` - Print integer
- `readch()` - Read character
- `readk()` - Read key
- `cls()` - Clear screen (bare-metal)
- `setcolor(attr)` - Set color (bare-metal)

---

### String (`string/`)

String manipulation.

```de
include "string.de"

fn main {
    <.de
        var msg: string = "Hello"
        var len: i32 = strlen(msg)
    .>
}
```

**Functions:**
- `strlen(str)` - Get string length
- `strcpy(dest, src)` - Copy string
- `strcmp(s1, s2)` - Compare strings
- `itoa(num, buf)` - Integer to string

---

### Test (`test/`)

Unit testing framework.

```de
include "test.de"

fn test_add() {
    <.de
        var result: i32 = 0
        result = (2 + 2)
        assert_eq(result, 4, "2 + 2 should equal 4")
    .>
}

fn main {
    <.de
        run_all_tests()
    .>
}
```

**Functions:**
- `assert_eq(actual, expected, msg)` - Assert equality
- `assert_true(cond, msg)` - Assert true
- `assert_false(cond, msg)` - Assert false
- `test_print_results()` - Print test summary
- `test_reset()` - Reset counters
- `run_all_tests()` - Run all registered tests

---

## Installation

### Using defo (recommended)

```bash
# Add to defo.json dependencies
{
    "dependencies": {
        "defacto-stdlib": "vivooifo-droid/Defacto"
    }
}

# Install
defo install
```

### Manual

```bash
# Clone repository
git clone https://github.com/vivooifo-droid/Defacto.git

# Copy stdlib
cp -r Defacto/stdlib/* your-project/lib/
```

### Using in code

```de
// Include specific module
include "core.de"
include "math.de"
include "io.de"

// Or include all
include "stdlib.de"
```

---

## Platform Support

| Module | Bare-metal | Linux | macOS | ARM64 |
|--------|-----------|-------|-------|-------|
| core | ✅ | ✅ | ✅ | ✅ |
| math | ✅ | ✅ | ✅ | ✅ |
| collections | ✅ | ✅ | ✅ | ✅ |
| io | ⚠️ Partial | ✅ | ✅ | ✅ |
| string | ✅ | ✅ | ✅ | ✅ |
| test | ✅ | ✅ | ✅ | ✅ |

---

## Testing

Run stdlib tests:

```bash
cd stdlib
defo test
```

---

## Contributing

1. Follow existing code style
2. Add tests for new functions
3. Update documentation
4. Test on all platforms

---

## License

Same license as Defacto compiler.
