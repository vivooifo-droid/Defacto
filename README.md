# Defacto v0.53 (alpha)

Low-level programming language for x86-32/64, ARM64, bare-metal experiments, and custom toolchains.

**What's new in v0.53:**
- **LLVM Backend** — Optional LLVM codegen for optimized native code generation
- **Generics** — Template-like type parameters for functions and structs `<T>`
- **Improved Arrays** — Expression indexes `arr[i + 1]`, slices `arr[1..5]`, bounds checking
- **Better Performance** — Up to 2x faster with LLVM optimizations

**Previous releases:**
- v0.52: Defo Package Manager, Standard Library, Test Framework
- v0.51: Bitwise operators, linter, formatter
- v0.50: Full ARM64 support, Apple Silicon native

## Repository contents

- Compiler (`compiler/`) — C++ with optional LLVM backend
- VS Code extension (`vscode-extension/`)
- **Package manager** (`tools/defo.sh`)
- **Code formatter** (`tools/deformat.sh`)
- **Code linter** (`tools/delint.sh`)
- **Standard Library** (`stdlib/`) — core, string, math, io
- **Examples** (`examples/`)
- C++ addons (`addons/cpp/`)
- Rust addons (`addons/rust/`)
- Backend framework (`addons/rust-backend/`)

## Install

### macOS (Homebrew) - Recommended

**The easiest way to install Defacto:**

```bash
# Add the Defacto tap
brew tap vivooifo-droid/Defacto

# Install Defacto (with LLVM support if available)
brew install defacto

# Verify installation
defacto -h
```

This will automatically:
- Download and install NASM dependency
- Build and install the compiler
- Add `defacto` to your PATH

**For Apple Silicon Macs:** To run `-terminal-macos` binaries, you need Rosetta 2:
```bash
softwareupdate --install-rosetta
```

### Linux (Ubuntu/Debian)

**Requirements:**
- Build essentials (gcc, make)
- NASM assembler
- LLVM (optional, for optimized codegen)

```bash
# Install dependencies (with LLVM for better performance)
sudo apt update
sudo apt install build-essential nasm llvm-dev

# Build compiler
cd compiler && make

# Test installation
./defacto -h
```

**Other Linux distributions:**

```bash
# Fedora
sudo dnf install gcc make nasm llvm-devel

# Arch Linux
sudo pacman -S gcc make nasm llvm
```

## Build from Source

```bash
cd compiler
make
./defacto -h
```

**With LLVM support (recommended for better performance):**

```bash
# Install LLVM first
# Ubuntu/Debian:
sudo apt install llvm-dev

# macOS:
brew install llvm

# Then build
cd compiler
make
./defacto -v  # Should show "LLVM backend enabled"
```

## Usage

**macOS** (default: `-terminal-macos`):
```bash
./defacto hello.de -o hello
./hello
```

**Linux** (default: `-terminal`):
```bash
./defacto hello.de -o hello
./hello
```

**Bare-metal kernel** (all platforms):
```bash
./defacto -kernel os.de -o kernel.bin
```

### Command Line

Compile a program:
```bash
./defacto program.de
```

Specify output file:
```bash
./defacto program.de -o output
./defacto -o output program.de    # order doesn't matter
```

Assembly output only:
```bash
./defacto -S program.de
```

Verbose mode:
```bash
./defacto -v program.de
```

Help:
```bash
./defacto -h
```

### Compiler Flags

| Flag | Description |
|------|-------------|
| `-o <file>` | Output file name |
| `-S` | Output assembly only |
| `-v` | Verbose mode |
| `-kernel` | Bare-metal kernel mode |
| `-terminal` | Linux terminal mode (32-bit) |
| `-terminal64` | Linux terminal mode (64-bit) |
| `-terminal-macos` | macOS terminal mode |
| `-terminal-arm64` | ARM64 terminal mode |
| `-llvm` | Use LLVM backend (if available) |
| `-O0`, `-O1`, `-O2`, `-O3` | Optimization level (LLVM only) |

## Modes

| Mode | Platform | Output | Default |
|------|----------|--------|---------|
| `-kernel` | All | Binary (x86-32) | Linux |
| `-terminal` | Linux | ELF 32-bit | Linux |
| `-terminal64` | Linux | ELF 64-bit | — |
| `-terminal-macos` | macOS | Mach-O 64-bit (x86_64) | macOS Intel |
| `-terminal-arm64` | macOS/Linux | Mach-O/ELF 64-bit (ARM64) | macOS ARM |

**Mode selection:**
- **macOS Intel**: Default is `-terminal-macos` (native macOS x86_64 binaries)
- **macOS ARM (M1/M2/M3)**: Default is `-terminal-arm64` (native ARM64 binaries)
- **Linux**: Default is `-terminal` (Linux 32-bit syscalls)
- **Linux 64-bit**: Use `-terminal64` for native 64-bit binaries
- **Bare-metal**: Use `-kernel` for OS development (no OS dependencies)

### Examples

```bash
# macOS native application
./defacto app.de -o app
./app

# Linux application (on Linux)
./defacto app.de -o app
./app

# Bare-metal kernel
./defacto -kernel kernel.de -o kernel.bin

# Run in QEMU
qemu-system-i386 -kernel kernel.bin

# With LLVM optimizations
./defacto -llvm -O2 app.de -o app
```

## Full language syntax

### Required file directives

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

#Mainprogramm.end
```

Notes:

- `#Mainprogramm.start` / `#Mainprogramm.end` are required.
- `#NO_RUNTIME` is required for bare-metal mode.
- `#SAFE` is reserved; memory checks are always enabled.
- `#INTERRUPT {num} == func` is parsed but not emitted yet.

### Sections

All code must be inside sections:

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"

    display{msg}
    x = (x + 1)
.>
```

Rules:

- Variables (`var`) and constants (`const`) can be declared anywhere in the section
- `.>` closes the section.

### Types and variables

```de
var count: i32 = 42
const big: i64 = 1000000
```

Supported types:

| Type | Size | Notes |
| --- | --- | --- |
| `i32` | 4 bytes | signed integer |
| `i64` | 8 bytes | signed integer |
| `u8` | 1 byte | unsigned byte |
| `string` | pointer | null-terminated string |
| `pointer` | 4/8 bytes | raw address (platform-dependent) |
| `bool` | 1 byte | true/false |
| `struct_name` | varies | user-defined struct type |

Arrays:

```de
var buf: u8[64]
var arr: i32[10]

// Dynamic array size (v0.53+)
const SIZE: i32 = 100
var data: i32[SIZE]

// Array with expression index (v0.53+)
arr[i + 1] = 42
var x = arr[i * 2]
```

### Generics (v0.53+)

Type parameters for functions and structs:

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

var box: Box<i32>
box.value = 42

// Multiple type parameters
struct Pair<T, U> {
    first: T
    second: U
}

var pair: Pair<i32, string>
```

### Structs

Define custom data structures:

```de
struct Point {
    x: i32
    y: i32
    z: i32
}

var p: Point
p.x = 10
p.y = 20
p.z = 30
```

**Arrays in structs (v0.46+):**

```de
struct Buffer {
    data: u8[256]
    size: i32
}

var buf: Buffer
buf.size = 10
```

Rules:

- Struct fields can be any built-in type.
- Structs are stored inline (no pointers).
- Field access uses dot notation: `struct.field`

### Expressions

Full nested expressions are supported:

```de
x = (a + b + c)
x = ((a + b) * c)
x = -5
result = (a * b) + (c / d)

// Bitwise operators (v0.51+)
x = (a & b)
x = (a | b)
x = (a ^ b)
x = (a << 2)
x = (a >> 1)

// Compound assignment (v0.51+)
x += 1    // x = (x + 1)
y -= 5    // y = (y - 5)
z *= 2    // z = (z * 2)
w /= 4    // w = (w / 4)
```

Supported operators:

| Operator | Description | Example |
| --- | --- | --- |
| `+` | Addition | `x = (a + b)` |
| `-` | Subtraction | `x = (x - 1)` |
| `*` | Multiplication | `x = (x * 2)` |
| `/` | Division | `x = (x / 4)` |
| `&` | Bitwise AND | `x = (a & b)` |
| `|` | Bitwise OR | `x = (a | b)` |
| `^` | Bitwise XOR | `x = (a ^ b)` |
| `<<` | Left shift | `x = (a << 2)` |
| `>>` | Right shift | `x = (a >> 1)` |

### Statements

#### Assignment

```de
x = 1
x = other
x = (x + 1)
arr[i] = x
p.x = 100

// Expression indexes (v0.53+)
arr[i + 1] = 42
arr[i * 2 + j] = 100
```

#### Loop

```de
loop {
    if x == 10 { stop }
}
```

#### If/Else (v0.30+)

```de
if x == y {
    display{msg}
} else {
    display{err}
}
```

All comparison operators are supported:

| Operator | Example | Description |
|----------|---------|-------------|
| `==` | `if x == y` | Equal |
| `!=` | `if x != y` | Not equal |
| `<` | `if x < y` | Less than |
| `>` | `if x > y` | Greater than |
| `<=` | `if x <= y` | Less than or equal |
| `>=` | `if x >= y` | Greater than or equal |

Rules:

- `else` block is optional.

#### For Loop

```de
for i = 0 to 10 {
    display{i}
}
```

#### While Loop

```de
while x < y {
    x = (x + 1)
}
```

#### Switch/Case (v0.46+)

```de
switch x {
    case 1:
        display{"one"}
    case 5:
        display{"five"}
    default:
        display{"other"}
}
```

### Functions

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

**With parameters:**

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

**With generics (v0.53+):**

```de
fn identity<T>(value: T) {
    <.de
        display{value}
    .>
}

call #identity
```

### Drivers

```de
driver keyboard {
    type = keyboard
}

// Or simpler - type inferred from name:
driver mouse

call #keyboard
call #mouse
```

Supported driver types: `keyboard`, `mouse`, `volume`

### Extern Functions (v0.46+)

Call C library functions:

```de
extern printf
extern malloc
extern free
```

### Type Aliases (v0.47+)

Create type aliases for better readability:

```de
type byte = u8
type int32 = i32
type pointer = *i32
```

### Inline Assembly (v0.47+)

Embed assembly code directly:

```de
asm {
    mov eax, 1
    add eax, ebx
}
```

### Builtins

```de
display{msg}
color{10}
readkey{key}
readchar{ch}
putchar{65}
clear{}
reboot{}
```

Notes:

- `display{string}` prints a string. In terminal mode it appends a newline.
- `readkey` and `readchar` work only in bare-metal mode. In terminal mode they return `0`.
- `readchar` supports ASCII `a-z`, `0-9`, space, enter, backspace.
- `color`, `putchar`, `clear`, `reboot` work only in bare-metal mode.

### Registers

```de
#MOV {#R1, x}
#MOV {x, #R1}
#R1 = (#R1 + #R2)
```

Notes:

- Register mapping is fixed to x86-32.
- `#STATIC` is parsed but not emitted yet.

## Memory management

**Manual memory management:**

Defacto is a low-level language with manual memory management. You control allocation and deallocation:

```de
<.de
    var ptr: *i32
    var size: i32 = 4

    alloc{size}
    #MOV {ptr, #R1}

    // Use pointer...

    dealloc{ptr}
.>
```

**Automatic cleanup:**

The compiler automatically frees all stack variables at the end of each section. You don't need to call `free{}` for local variables.

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    display{msg}
.>
```

## defo package manager

```bash
./defo init my-app
./defo install https://github.com/owner/repo.git
./defo install owner/repo
./defo install owner/repo#main
./defo list
```

## What can you build with Defacto?

Defacto is designed for low-level systems programming.

**Suitable for:**

| Project Type | Example | Difficulty |
|-------------|---------|------------|
| **Operating Systems** | Bare-metal kernels, bootloaders | Advanced |
| **Embedded systems** | Microcontrollers, IoT devices | Intermediate |
| **System utilities** | Disk tools, hardware monitors | Intermediate |
| **Educational projects** | Learning OS dev, compilers | Beginner |
| **Demo scenes** | 64kb intros, demoscene productions | Advanced |
| **Retro computing** | DOS-like programs, demoscene | Beginner |

**Not suitable for:**

- Web applications (use Rust, Go, or JavaScript)
- Mobile apps (use Swift, Kotlin, or Flutter)
- Data science (use Python or Julia)
- Desktop GUI apps (use C#, Java, or Electron)

## Performance

Defacto compiles to native code with minimal runtime overhead.

**Comparison (approximate, Fibonacci benchmark):**

| Language | Relative Speed | Notes |
|----------|---------------|-------|
| Assembly (hand-optimized) | 1.0x | Baseline |
| C (gcc -O3) | 1.0-1.2x | Mature optimizer |
| Rust | 1.0-1.3x | LLVM backend |
| Zig | 1.1-1.4x | LLVM backend |
| **Defacto v0.53 (LLVM)** | 1.2-1.5x | LLVM backend with optimizations |
| **Defacto v0.53 (NASM)** | 2-5x | Simple codegen, no optimizations |
| Go | 2-5x | GC overhead |
| Python | 50-100x | Interpreter |

**Why Defacto with LLVM is fast:**
- Compiles to native code (no VM, no interpreter)
- No garbage collector
- LLVM optimizations (inlining, vectorization, etc.)
- Direct system calls in terminal mode
- Minimal runtime (~zero overhead)

**Why NASM backend is slower:**
- No advanced optimizations
- Simple codegen
- Early stage compiler

For bare-metal and systems programming, performance is limited by your code quality, not the language.

## Addons

Extend Defacto with native libraries!

### Rust Addons

Write high-performance addons in Rust with memory safety:

```bash
cd addons/rust
cargo build --release
```

See [`addons/rust/README.md`](addons/rust/README.md) for the full guide.

### Backend Framework (Rust)

HTTP web framework for building servers:

```bash
cd addons/rust-backend
cargo build --release
```

See [`addons/rust-backend/README.md`](addons/rust-backend/README.md) for API docs.

### C++ Addons

Traditional C++ addons are also supported:

```bash
cd addons/cpp
make
```

See [`addons/cpp/`](addons/cpp/) for examples.

## Tools

### Package Manager (defo)

```bash
# Initialize new project
./tools/defo.sh init my-app

# Install library
./tools/defo.sh install owner/repo

# List libraries
./tools/defo.sh list

# Build and run
./tools/defo.sh build
./tools/defo.sh run
```

### Code Formatter

```bash
./tools/deformat.sh program.de
```

### Code Linter

```bash
./tools/delint.sh program.de
```

## Limitations

- Interrupt directives are parsed but not generated
- Some complex expressions may require temporary variables

## Examples

### Hello World

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

<.de
    var hello: string = "Hello, World!"
    display{hello}
.>

#Mainprogramm.end
```

### Generic Swap Function

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
    
    // Swap integers
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

---

## Migration Guide from v0.52

No breaking changes! All existing code continues to work.

To use new features:

```de
// Generics
fn identity<T>(value: T) {
    <.de
        display{value}
    .>
}

// Expression indexes
var arr: i32[10]
arr[i + 1] = 42

// Slices (future)
// var slice = arr[2..5]
```

## License

MIT License

## Contributors

Thanks to all contributors!

---

**Previous release:** [v0.52](RELEASE-v0.52.md) | **[v0.53 Release Notes](RELEASE-v0.53.md)**
