# Defacto v0.45 Release Notes

**Release Date:** February 28, 2026
**Status:** Alpha

## Breaking Changes

### Old Syntax Removed

v0.45 removes all deprecated syntax from v0.44 and earlier. This is a breaking change.

### Removed Features

| Feature | Old Syntax | New Syntax |
|---------|-----------|------------|
| Functions | `function == name { }` | `fn name { }` |
| Sections | `static.pl>` separator | Not needed |
| For loops | `for i = 0; i < 10; i = (i + 1) { }` | `for i = 0 to 10 { }` |
| Drivers | `<drv. ... .dr>` | `driver name { type = ... }` |

## Migration Guide

### 1. Update Functions

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

### 2. Remove `static.pl>`

**Before:**
```de
<.de
    var x: i32 = 0
    static.pl>
    display{x}
.>
```

**After:**
```de
<.de
    var x: i32 = 0
    display{x}
.>
```

### 3. Update For Loops

**Before:**
```de
for i = 0; i < 10; i = (i + 1) {
    display{i}
}
```

**After:**
```de
for i = 0 to 10 {
    display{i}
}
```

### 4. Update Drivers

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

## What's Changed

### Compiler Changes

- **lexer.h**: Removed `TT::FUNCTION`, `TT::STATIC_PL`, `TT::DRV_OPEN`, `TT::DRV_CLOSE`
- **parser.h**: 
  - Removed `parse_driver_section()` for old driver syntax
  - Simplified `parse_function()` - only `fn` keyword
  - Simplified `parse_section()` - no `static.pl>` support
  - Simplified `for` parser - only `to` syntax
- **codegen.h**: Removed `gen_driver_section()` for old driver syntax

### Code Cleanup

- Removed ~1000 lines of legacy parser code
- Simplified error messages
- Faster compilation

## Examples

### Hello World

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var msg: string = "Hello, World!"
    display{msg}
.>

#Mainprogramm.end
```

### Functions with Parameters

```de
fn add(a: i32, b: i32) {
    <.de
        var result: i32 = 0
        result = (a + b)
        display{result}
    .>
}

call #add
```

### For Loop

```de
for i = 0 to 10 {
    display{i}
}
```

### Drivers

```de
driver keyboard {
    type = keyboard
}

fn main {
    <.de
        var key: i32 = 0
        call #keyboard
        readkey{key}
    .>
}
```

## Upgrade Path

If you have existing code:

1. Update all `function ==` to `fn`
2. Remove all `static.pl>` lines
3. Update all `for` loops to use `to`
4. Update all driver declarations

**Automated migration:** Consider writing a simple script to:
- Replace `function ==` with `fn`
- Remove `static.pl>` lines
- Convert old `for` syntax (complex - may need manual review)

## Known Issues

- Expression indexes in arrays still not supported: `arr[i + 1]` ❌
- Interrupt directives still not generated (parsed only)

---

**Previous release:** [v0.44](RELEASE-v0.44.md)
