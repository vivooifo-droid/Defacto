# Defacto v0.44 Release Notes

**Release Date:** February 28, 2026

## Major Changes

### 🎉 No more `static.pl>` separator

Declarations and statements can now be mixed freely within sections. The `static.pl>` separator is **optional** for backward compatibility.

**Before (v0.43):**
```de
<.de
    var x: i32 = 0
    var msg: string = "hello"

    static.pl>

    display{msg}
    x = (x + 1)
.>
```

**After (v0.44):**
```de
<.de
    var x: i32 = 0
    var msg: string = "hello"

    display{msg}
    x = (x + 1)
.>
```

### 🔧 Simplified Function Syntax

New `fn` keyword for cleaner function declarations:

**Before:**
```de
function == my_func {
    <.de
        var a: i32 = 0
        static.pl>
        a = (a + 1)
    .>
}
```

**After:**
```de
fn my_func {
    <.de
        var a: i32 = 0
        a = (a + 1)
    .>
}
```

**With parameters:**
```de
fn add(a: i32, b: i32) {
    <.de
        var result: i32 = 0
        result = (a + b)
    .>
}
```

### 🔁 For Loop with `to` Keyword

New simplified syntax for counting loops:

**New syntax:**
```de
for i = 0 to 10 {
    display{i}
}
```

**Old syntax (still supported):**
```de
for i = 0; i < 10; i = (i + 1) {
    display{i}
}
```

### ⚖️ Full Comparison Operators

All comparison operators now work correctly in `if` and `while` conditions:

| Operator | Example | Description |
|----------|---------|-------------|
| `==` | `if x == y` | Equal |
| `!=` | `if x != y` | Not equal |
| `<` | `if x < y` | Less than |
| `>` | `if x > y` | Greater than |
| `<=` | `if x <= y` | Less than or equal |
| `>=` | `if x >= y` | Greater than or equal |

**Example:**
```de
if x != y {
    display{msg}
}

while count <= max {
    count = (count + 1)
}
```

### 🖥️ Simplified Driver Syntax

New clean driver declarations:

**Before:**
```de
<drv.
Const.driver = keyboard_driver
keyboard_driver <<func = keyboard>>
.dr>

call #keyboard_driver
```

**After:**
```de
driver keyboard {
    type = keyboard
}

call #keyboard
```

**Or even simpler:**
```de
driver keyboard

call #keyboard
```

**Custom name with explicit type:**
```de
driver my_keyboard {
    type = keyboard
}

call #my_keyboard
```

## Backward Compatibility

✅ **100% backward compatible** - All existing code continues to work:
- Old `function == name { }` syntax still supported
- `static.pl>` separator still works if you prefer it
- Old `for` loop syntax with semicolons still supported

## What's Changed

### Compiler Changes

- **lexer.h**: Added `TT::FN`, `TT::TO`, `TT::DRIVER_KEYWORD`, `TT::TYPE` tokens
- **parser.h**:
  - Updated `parse_function()` to support both old and new syntax
  - Updated `parse_section()` to allow mixed declarations/statements
  - Enhanced `for` loop parser for `to` keyword
  - Added `parse_driver()` for new driver syntax
- **defacto.h**: Added `DriverDecl` struct and `TT::DRIVER_KEYWORD` token
- **codegen.h**: Added `gen_driver()` for new driver syntax

### Documentation

- Updated README.md with new syntax examples
- Updated SYNTAX.md with complete driver documentation
- All examples now show both old and new syntax where applicable

## Migration Guide

### Optional: Migrate to New Syntax

You don't need to change anything, but here's how to adopt new features:

**1. Remove `static.pl>` (optional):**
```de
# Before
<.de
    var x: i32 = 0
    static.pl>
    display{x}
.>

# After
<.de
    var x: i32 = 0
    display{x}
.>
```

**2. Use `fn` for functions:**
```de
# Before
function == my_func {
    <.de ... .>
}

# After
fn my_func {
    <.de ... .>
}
```

**3. Simplify `for` loops:**
```de
# Before
for i = 0; i < 10; i = (i + 1) { ... }

# After
for i = 0 to 10 { ... }
```

**4. Use new driver syntax:**
```de
# Before
<drv.
Const.driver = keyboard_driver
keyboard_driver <<func = keyboard>>
.dr>

# After
driver keyboard {
    type = keyboard
}

# Or even simpler:
driver keyboard
```

## Known Issues

- Expression indexes in arrays still not supported: `arr[i + 1]` ❌
- Interrupt directives still not generated (parsed only)

## Contributors

Thanks to all users who provided feedback on v0.43!

## Download

See [DOWNLOAD.md](DOWNLOAD.md) for installation instructions.

---

**Previous release:** [v0.43](RELEASE-v0.43.md)
