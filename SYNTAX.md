# Defacto Language Syntax Reference

Version 0.53

## Table of Contents

1. [File Structure](#file-structure)
2. [Compiler Directives](#compiler-directives)
3. [Code Sections](#code-sections)
4. [Types](#types)
5. [Variables and Constants](#variables-and-constants)
6. [Generics](#generics)
7. [Structs](#structs)
8. [Expressions](#expressions)
9. [Statements](#statements)
10. [Functions](#functions)
11. [Drivers](#drivers)
12. [Built-in Functions](#built-in-functions)
13. [Registers](#registers)
14. [Comments and Strings](#comments-and-strings)
15. [Memory Management](#memory-management)
16. [Language Limitations](#language-limitations)

---

## File Structure

Every Defacto file must have this structure:

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

struct MyStruct {
    field: i32
}

driver keyboard {
    type = keyboard
}

fn my_func {
    <.de
        var x: i32 = 0
        x = (x + 1)
    .>
}

<.de
    var msg: string = "Hello"
    display{msg}
.>

#Mainprogramm.end
```

### Element Order

1. Directives (`#NO_RUNTIME`, `#SAFE`)
2. Structs (`struct`)
3. Drivers (`driver`)
4. Functions (`fn`)
5. Main section (`<.de` ... `.>`)
6. Closing directive (`#Mainprogramm.end`)

---

## Compiler Directives

### `#Mainprogramm.start`

Required. Must be the first line.

### `#Mainprogramm.end`

Required. Must be the last line.

### `#NO_RUNTIME`

Required for bare-metal mode. Disables standard runtime.

### `#SAFE`

Reserved. Memory checks always enabled.

---

## Code Sections

### Main Section `<.de` ... `.>`

All executable code must be inside sections:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"

    display{msg}
    x = (x + 1)

    var y: i32 = 10
    y = (y * 2)
.>
```

Rules:

- Variables (`var`) and constants (`const`) can be declared anywhere
- Statements and declarations can be mixed
- `.>` closes the section

---

## Types

### Built-in Types

| Type | Size | Description |
|------|------|-------------|
| `i32` | 4 bytes | Signed integer |
| `i64` | 8 bytes | Signed integer |
| `u8` | 1 byte | Unsigned byte |
| `string` | pointer | Null-terminated string |
| `pointer` | 4/8 bytes | Raw pointer (platform-dependent) |
| `bool` | 1 byte | Boolean (true/false) |

### Arrays

```de
var buf: u8[64]
var arr: i32[10]

// Dynamic size with constant (v0.53+)
const SIZE: i32 = 100
var data: i32[SIZE]
```

### Pointers

```de
var ptr: *i32
var ptr: *string
var ptr: **i32
```

---

## Variables and Constants

### Variable Declaration

```de
var count: i32 = 42
var msg: string = "hello"
var ptr: *i32 = &count
```

### Constants

```de
const MAX: i32 = 100
const NAME: string = "Defacto"
```

Rules:
- `const` must have an initializer
- `const` cannot be modified
- `const` cannot be freed

---

## Generics (v0.53+)

Type parameters allow writing reusable code that works with multiple types.

### Generic Functions

```de
fn swap<T>(a: T, b: T) {
    <.de
        var temp: T = a
        a = b
        b = temp
    .>
}

fn identity<T>(value: T) {
    <.de
        display{value}
    .>
}
```

### Generic Structs

```de
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

### Type Constraints (Future)

```de
// Future syntax - not yet implemented
fn compare<T: comparable>(a: T, b: T) {
    <.de
        if a == b {
            display{"equal"}
        }
    .>
}
```

---

## Structs

### Definition

```de
struct Point {
    x: i32
    y: i32
    z: i32
}
```

### Usage

```de
var p: Point
p.x = 10
p.y = 20
p.z = 30
```

### Pointer to Struct

```de
var ptr: *Point = &p
ptr->x = 100
```

### Generic Structs

```de
struct Container<T> {
    data: T
    size: i32
}

var container: Container<i32>
```

---

## Expressions

### Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `x = (a + b)` |
| `-` | Subtraction | `x = (x - 1)` |
| `*` | Multiplication | `x = (x * 2)` |
| `/` | Division | `x = (x / 4)` |
| `&` | Bitwise AND | `x = (a & b)` |
| `|` | Bitwise OR | `x = (a | b)` |
| `^` | Bitwise XOR | `x = (a ^ b)` |
| `<<` | Left shift | `x = (a << 2)` |
| `>>` | Right shift | `x = (a >> 1)` |

### Nested Expressions

```de
x = (a + b + c)
x = ((a + b) * c)
x = -5
result = (a * b) + (c / d)
```

### Compound Assignment (v0.51+)

```de
x += 1    // x = (x + 1)
y -= 5    // y = (y - 5)
z *= 2    // z = (z * 2)
w /= 4    // w = (w / 4)
```

---

## Statements

### Assignment

```de
x = 1
x = other
x = (x + 1)
arr[i] = x
p.x = 100
*ptr = value

// Expression indexes (v0.53+)
arr[i + 1] = 42
arr[i * 2 + j] = 100
```

### If/Else

```de
if x == y {
    display{msg}
} else {
    display{err}
}
```

### Comparison Operators

| Operator | Example | Description |
|----------|---------|-------------|
| `==` | `if x == y` | Equal |
| `!=` | `if x != y` | Not equal |
| `<` | `if x < y` | Less than |
| `>` | `if x > y` | Greater than |
| `<=` | `if x <= y` | Less than or equal |
| `>=` | `if x >= y` | Greater than or equal |

### For Loop

```de
for i = 0 to 10 {
    display{i}
}
```

### While Loop

```de
while x < y {
    x = (x + 1)
}
```

### Loop

```de
loop {
    if x == 10 { stop }
}
```

### Stop

Exit loop or program:

```de
loop {
    if x == 10 { stop }
}
```

### Continue

Skip to next iteration:

```de
loop {
    if x == 0 { continue }
    x = (x - 1)
}
```

### Return

Return from function:

```de
return{value}
```

---

## Functions

### Declaration

```de
fn my_func {
    <.de
        var a: i32 = 0
        a = (a + 1)
        display{a}
    .>
}

call #my_func
```

### With Parameters

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

### With Generics (v0.53+)

```de
fn identity<T>(value: T) {
    <.de
        display{value}
    .>
}

fn swap<T>(a: T, b: T) {
    <.de
        var temp: T = a
        a = b
        b = temp
    .>
}

call #identity
call #swap
```

---

## Drivers

### Declaration

```de
driver keyboard {
    type = keyboard
}

call #keyboard
```

### Simplified

```de
driver mouse

call #mouse
```

### Custom Name

```de
driver my_volume {
    type = volume
}

call #my_volume
```

Supported driver types: `keyboard`, `mouse`, `volume`

---

## Built-in Functions

### I/O

| Function | Description |
|----------|-------------|
| `display{var}` | Print string |
| `readkey{var}` | Read scancode (bare-metal) |
| `readchar{var}` | Read ASCII (bare-metal) |
| `putchar{code}` | Print character (bare-metal) |

### Graphics (Bare-metal)

| Function | Description |
|----------|-------------|
| `color{attr}` | Set color attribute |
| `clear{}` | Clear screen |

### System

| Function | Description |
|----------|-------------|
| `reboot{}` | Reboot system |

---

## Registers

### Available Registers

| Register | x86 |
|----------|-----|
| `#R1` | EDI |
| `#R2` | ESI |
| `#R3` | EDX |
| `#R4` | ECX |
| `#R5` | EBX |
| `#R6` | EAX |
| `#R15` | EBP |
| `#R16` | ESP |

### Operations

```de
#MOV {#R1, x}
#MOV {x, #R1}
#R1 = (#R1 + #R2)
```

---

## Comments and Strings

### Comments

```de
// This is a comment
var x: i32 = 0
```

### Strings

```de
var msg: string = "Hello, World!"
var multiline: string = "Line 1\nLine 2"
```

### Escape Sequences

| Sequence | Character |
|----------|-----------|
| `\n` | Newline |
| `\t` | Tab |

---

## Memory Management

### Automatic

Variables are automatically freed at end of section:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    display{msg}
.>
```

### Manual (Legacy)

```de
free{x}
```

### Dynamic Allocation

```de
var ptr: *i32
var size: i32 = 4

static.pl>

alloc{size}
#MOV {ptr, #R1}

dealloc{ptr}
```

---

## Language Limitations

- Interrupt directives parsed but not generated
- Expression indexes in arrays: `arr[i + 1]` supported (v0.53+)
- No nested struct definitions inside sections
- Generics require explicit type specification

---

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

### Calculator

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var a: i32 = 10
    var b: i32 = 5
    var result: i32 = 0

    result = (a + b)
    result = (a - b)
    result = (a * b)
    result = (a / b)
.>

#Mainprogramm.end
```

### Generic Swap

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

fn swap<T>(a: T, b: T) {
    <.de
        var temp: T = a
        a = b
        b = temp
    .>
}

<.de
    var x: i32 = 10
    var y: i32 = 20
    
    call #swap
    
    display{x}
    display{y}
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
    
    // Access with expression index
    var x: i32 = arr[i - 1]
    display{x}
.>

#Mainprogramm.end
```

### Generic Box

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
    display{int_box.value}
    
    var str_box: Box<string>
    str_box.value = "Hello"
    display{str_box.value}
.>

#Mainprogramm.end
```

---

## Compilation

```bash
# Compile
./defacto program.de

# Specify output
./defacto program.de -o output

# With LLVM backend
./defacto -llvm -O2 program.de -o output

# Assembly output only
./defacto -S program.de

# Verbose
./defacto -v program.de

# Help
./defacto -h
```

### Modes

| Mode | Platform | Output |
|------|----------|--------|
| `-kernel` | All | Binary (x86-32) |
| `-terminal` | Linux | ELF 32-bit |
| `-terminal64` | Linux | ELF 64-bit |
| `-terminal-macos` | macOS | Mach-O 64-bit |
| `-terminal-arm64` | macOS/Linux | Mach-O/ELF 64-bit (ARM64) |

### Optimization Levels (LLVM only)

| Flag | Description |
|------|-------------|
| `-O0` | No optimization (debug) |
| `-O1` | Basic optimizations |
| `-O2` | Standard optimizations |
| `-O3` | Aggressive optimizations |
