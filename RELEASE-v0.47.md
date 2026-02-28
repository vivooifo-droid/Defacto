# Defacto v0.47 Release Notes

**Release Date:** February 28, 2026
**Status:** Alpha

## New Features

### Type Aliases

Create type aliases for better code readability:

```de
type byte = u8
type int32 = i32
type pointer = *i32
```

### Compound Assignment

Shorter assignment operators:

```de
x += 1    // x = (x + 1)
y -= 5    // y = (y - 5)
z *= 2    // z = (z * 2)
w /= 4    // w = (w / 4)
```

### Inline Assembly

Embed assembly code directly:

```de
asm {
    mov eax, 1
    mov ebx, 2
    add eax, ebx
}
```

### Switch/Case (from v0.46)

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

### Arrays in Structs (from v0.46)

```de
struct Buffer {
    data: u8[256]
    size: i32
}
```

## Example

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE

type byte = u8

struct Buffer {
    data: u8[256]
    size: i32
}

extern printf

fn main {
    <.de
        var x: byte = 5
        x += 3
        
        switch x {
            case 1:
                display{"one"}
            case 8:
                display{"eight"}
            default:
                display{"other"}
        }
        
        asm {
            mov eax, 1
        }
    .>
}

#Mainprogramm.end
```

## Compiler Changes

- **lexer.h**: Added `PLUS_EQ`, `MINUS_EQ`, `MUL_EQ`, `DIV_EQ`, `TYPE`, `ASM` tokens
- **parser.h**: Added `parse_type_alias()`, `parse_asm()`, compound assignment support
- **codegen.h**: Added `gen_type_alias()`, `gen_asm()`
- **defacto.h**: Added `TypeAliasNode`, `AsmStmtNode`

## Known Issues

- Bitwise operators (&, |, ^) not yet supported in expressions
- Switch only supports simple values (no expressions in cases)

---

**Previous release:** [v0.46](RELEASE-v0.46.md)
