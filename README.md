# Defacto (pre-alpha)

Low-level programming language for x86-32, bare-metal experiments, and custom toolchains.
Pre-alpha status: the language and toolchain are unstable and will change.

Русская версия: `README.ru.md`
Full syntax and details: `DOCS.md`

## Repository contents

- Compiler (`compiler/`)
- VS Code extension (`vscode-extension/`)
- Naive package manager `defo`
- C++ addons sandbox (`addons/`)

## Install

macOS:

```
xcode-select --install
brew install nasm
```

Linux (Ubuntu/Debian):

```
sudo apt update
sudo apt install build-essential nasm
```

Windows (native build with MSYS2):

1. Install MSYS2.
2. Open **MSYS2 MINGW64** terminal.
3. Install tools:

```
pacman -S --needed mingw-w64-x86_64-toolchain make nasm git
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

## Full language syntax (summary)

### Required file directives

```
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

```
<.de
    var x: i32 = 0
    var msg: string = "hello"

    static.pl>

    display{msg}
    x = (x + 1)
    free{x}
    free{msg}
.>
```

Rules:

- Before `static.pl>` only `var` and `const` are allowed.
- After `static.pl>` only statements are allowed.
- `.>` closes the section.

### Types and variables

```
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

Arrays:

```
var buf: u8[64]
var arr: i32[10]
```

Rules:

- `const` arrays are not supported.
- `const` values must be initialized.

### Expressions

Only simple expressions are supported:

```
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

Assignment:

```
x = 1
x = other
x = (x + 1)
arr[i] = x
```

Loop:

```
loop {
    if x == 10 { stop }
}
```

If:

```
if x == y { stop }
```

Rules:

- Only `==` is supported.
- Left and right sides must be a single token (number, variable, register).
- No else/elseif.

### Functions

```
function == my_func {
    <.de
        var a: i32 = 0
        static.pl>
        a = (a + 1)
        free{a}
    .>
}

call #my_func
```

### Builtins

```
display{msg}
free{x}
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

```
#MOV {#R1, x}
#MOV {x, #R1}
#R1 = (#R1 + #R2)
```

Notes:

- Register mapping is fixed to x86-32.
- `#STATIC` is parsed but not emitted yet.

## Memory rules

- Every `var` must be freed with `free{}`.
- `const` cannot be freed.
- Strings and arrays allocated as `var` must be freed.

## Comments and strings

- Line comments use `//`.
- Strings support `\n` and `\t` escapes.

## defo package manager

```
./defo init my-app
./defo install https://github.com/owner/repo.git
./defo install owner/repo
./defo install owner/repo#main
./defo list
```

##  limitations

- Only `==` in `if`.
- No else/elseif.
- One operator per expression.
- No negative number literals.
- No expression indexes in arrays.
- Interrupt directives are parsed but not generated.
