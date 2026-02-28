# Defacto v0.45 (alpha)

Low-level programming language for x86-32, bare-metal experiments, and custom toolchains.

**What's new in v0.45:**
- Clean syntax only - old deprecated syntax removed
- No backward compatibility - only new fn, driver, for..to syntax
- Simplified codebase - removed legacy parser code

## Repository contents

- Compiler (`compiler/`)
- VS Code extension (`vscode-extension/`)
- Naive package manager `defo`
- C++ addons sandbox (`addons/cpp/`)
- **Rust addons** (`addons/rust/`) - Write libraries in Rust!
- **Backend framework** (`addons/rust-backend/`) - HTTP web framework for Defacto

## Install

### macOS (Homebrew) - Recommended

**The easiest way to install Defacto:**

```bash
# Add the Defacto tap
brew tap vivooifo-droid/Defacto

# Install Defacto
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

**Note:** Linux uses `-terminal` by default, producing ELF 32-bit binaries linked with libc.

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential nasm

# Build compiler
cd compiler && make

# Test installation
./defacto -h
```

**Other Linux distributions:**

```bash
# Fedora
sudo dnf install gcc make nasm

# Arch Linux
sudo pacman -S gcc make nasm
```

## Build from Source

See installation instructions above for your platform, or:

```bash
cd compiler
make
./defacto -h
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

## Modes

| Mode | Platform | Output | Default |
|------|----------|--------|---------|
| `-kernel` | All | Binary (x86-32) | Linux |
| `-terminal` | Linux | ELF 32-bit | Linux |
| `-terminal-macos` | macOS | Mach-O 64-bit | macOS |

**Mode selection:**
- **macOS**: Default is `-terminal-macos` (native macOS binaries)
- **Linux**: Default is `-terminal` (Linux syscalls)
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
| `pointer` | 4 bytes | raw address |
| `struct_name` | varies | user-defined struct type |

Arrays:

```de
var buf: u8[64]
var arr: i32[10]
```

Rules:

- `const` arrays are not supported.
- `const` values must be initialized.

### Structs (v0.30+)

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
```

Supported operators:

| Operator | Description | Example |
| --- | --- | --- |
| `+` | Addition | `x = (a + b)` |
| `-` | Subtraction | `x = (x - 1)` |
| `*` | Multiplication | `x = (x * 2)` |
| `/` | Division | `x = (x / 4)` |

### Statements

#### Assignment

```de
x = 1
x = other
x = (x + 1)
arr[i] = x
p.x = 100
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

#### Loop

```de
loop {
    if x == 10 { stop }
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

**Automatic memory management (v0.30+):**

The compiler automatically frees all variables at the end of each section. You don't need to call `free{}` manually.

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    static.pl>
    display{msg}
.>
```

Legacy `free{}` is still supported for backward compatibility.

## Comments and strings

- Line comments use `//`.
- Strings support `\n` and `\t` escapes.

```de
var msg: string = "Hello\nWorld"
```

## defo package manager

```
./defo init my-app
./defo install https://github.com/owner/repo.git
./defo install owner/repo
./defo install owner/repo#main
./defo list
```

## Limitations

- No expression indexes in arrays: `arr[i + 1]` not supported
- Interrupt directives are parsed but not generated

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

## What's new in v0.35

- **Pointers** — Rust-like pointer syntax with `*Type`, `&var`, `*ptr`, `ptr->field`
- **System allocator** — malloc/free for dynamic memory in terminal mode
- **Null pointers** — `null` keyword for null pointer values
- **Double pointers** — `**Type` for pointer-to-pointer
- **Address-of operator** — `&` for taking variable addresses
- **Dereference operator** — `*` for pointer dereferencing
- **Arrow operator** — `->` for struct field access through pointers

## What's new in v0.30

- **Automatic memory management** - No more `free{}` required!
- **if/else statements** - Full conditional branching support
- **Struct types** - Define custom data structures with fields
- **Field access** - Access struct fields with dot notation
- **Improved Windows installer** - Simplified installation process
