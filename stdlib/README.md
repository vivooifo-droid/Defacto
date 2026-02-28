# Defacto Standard Library

Standard library for Defacto programming language.

## Modules

### Core (`core/`)
Basic types and constants:
- Type aliases (`byte`, `word`, `dword`, `qword`, `ptr`)
- Constants (`TRUE`, `FALSE`, `NULL`)
- Limits (`I32_MIN`, `I32_MAX`, `U8_MAX`)

**Usage:**
```de
include "core.de"

fn main {
    <.de
        var x: byte = 10
        var ptr: ptr = null
    .>
}
```

### String (`string/`)
String manipulation:
- `strlen(str)` - Get string length
- `strcpy(dest, src)` - Copy string
- `strcmp(s1, s2)` - Compare strings
- `itoa(num, buf)` - Integer to string

**Usage:**
```de
include "string.de"

fn main {
    <.de
        var msg: string = "Hello"
        var len: i32 = 0
        len = strlen(msg)
    .>
}
```

### Math (`math/`)
Mathematical functions:
- `abs(x)` - Absolute value
- `min(a, b)` - Minimum
- `max(a, b)` - Maximum
- `pow(base, exp)` - Power
- `sqrt(x)` - Square root
- `mod(a, b)` - Modulo

**Usage:**
```de
include "math.de"

fn main {
    <.de
        var x: i32 = -5
        var y: i32 = 0
        y = abs(x)  // y = 5
    .>
}
```

### I/O (`io/`)
Input/Output functions:
- `println(msg)` - Print string with newline
- `printch(ch)` - Print character
- `printi(num)` - Print integer
- `readch()` - Read character
- `readk()` - Read key
- `cls()` - Clear screen
- `setcolor(attr)` - Set color

**Usage:**
```de
include "io.de"

fn main {
    <.de
        println("Hello, World!")
        var ch: i32 = readch()
    .>
}
```

## Installation

The standard library is included with the Defacto compiler.

## Platform Support

| Module | Bare-metal | Linux | macOS |
|--------|-----------|-------|-------|
| core | ✅ | ✅ | ✅ |
| string | ✅ | ✅ | ✅ |
| math | ✅ | ✅ | ✅ |
| io | ⚠️ Partial | ✅ | ✅ |

Note: Some I/O functions (`cls`, `setcolor`) only work in bare-metal mode.

## Contributing

To contribute to the standard library:
1. Follow the existing code style
2. Add documentation comments
3. Test on all platforms
4. Submit a pull request

## License

Same license as Defacto compiler.
