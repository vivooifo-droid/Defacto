# Defacto v0.30 (pre-alpha)

Low-level programming language for x86-32, bare-metal experiments, and custom toolchains.
Pre-alpha status: the language and toolchain are unstable and will change.

Русская версия: `README.ru.md`

## Repository contents

- Compiler (`compiler/`)
- VS Code extension (`vscode-extension/`)
- Naive package manager `defo`
- C++ addons sandbox (`addons/cpp/`)
- **Rust addons** (`addons/rust/`) - Write libraries in Rust!
- **Backend framework** (`addons/rust-backend/`) - HTTP web framework for Defacto

## Install

### Windows (Recommended)

Download and run the installer: [defacto-0.30-installer.exe](https://github.com/vivooifo-droid/Defacto/releases/download/v0.30/defacto-0.30-installer.exe)

The installer will:
- Install the Defacto compiler to `C:\Program Files\Defacto`
- Add Defacto to your PATH
- Create Start Menu shortcuts
- Include example files

**Requirements:** Install NASM first:
- Download from: https://www.nasm.us/
- Or use Chocolatey: `choco install nasm`

### macOS

```
xcode-select --install
brew install nasm mingw-w64
cd compiler && make
./defacto -h
```

### Linux (Ubuntu/Debian)

```
sudo apt update
sudo apt install build-essential nasm
cd compiler && make
./defacto -h
```

## Build compiler

```
cd compiler
make
./defacto -h
```

## Usage

Compile a program:

```
./defacto program.de
```

Output file:

```
./defacto -o output.bin program.de
```

Assembly only:

```
./defacto -S program.de
```

Verbose:

```
./defacto -v program.de
```

## Modes

- `-kernel` default bare-metal (x86-32)
- `-terminal` Linux syscalls
- `-terminal-macos` macOS syscalls (Mach-O x86_64)

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

    static.pl>

    display{msg}
    x = (x + 1)
.>
```

Rules:

- Before `static.pl>` only `var` and `const` are allowed.
- After `static.pl>` only statements are allowed.
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

Only simple expressions are supported:

```de
x = (a + b)
x = (x - 1)
x = (x * 2)
x = (x / 4)
```

Limitations:

- Exactly one operator per expression.
- No nested expressions like `(a + b + c)`.
- Negative literals are not supported. Use `(0 - 1)` or a temp variable.
- Array access in expressions must use `arr[idx]` with `idx` as number, variable, or register.

### Statements

#### Assignment

```de
x = 1
x = other
x = (x + 1)
arr[i] = x
p.x = 100        // struct field
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

Rules:

- Only `==` is supported for comparison.
- Left and right sides must be a single token (number, variable, register).
- `else` block is optional.

### Functions

```de
function == my_func {
    <.de
        var a: i32 = 0
        static.pl>
        a = (a + 1)
    .>
}

call #my_func
```

### Drivers

Define and use hardware drivers:

```de
<drv.
Const.driver = keyboard_driver
keyboard_driver <<func = keyboard>>
.dr>

// Use the driver
call #keyboard_driver
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
// x and msg are automatically freed here
```

Legacy `free{}` is still supported for backward compatibility.

## Comments and strings

- Line comments use `//`.
- Strings support `\n` and `\t` escapes.

```de
// This is a comment
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

- Only `==` in `if` conditions.
- One operator per expression.
- No negative number literals.
- No expression indexes in arrays.
- Interrupt directives are parsed but not generated.

## Addons

Extend Defacto with native libraries!

### Rust Addons (Recommended)

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

## What's new in v0.30

- **Automatic memory management** - No more `free{}` required!
- **if/else statements** - Full conditional branching support
- **Struct types** - Define custom data structures with fields
- **Field access** - Access struct fields with dot notation
- **Improved Windows installer** - Simplified installation process
