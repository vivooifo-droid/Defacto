# Defacto v0.30 Release Notes

## 🎉 Major Features

### Automatic Memory Management

**The compiler now automatically frees all variables!** You no longer need to call `free{}` manually.

```de
<.de
    var x: i32 = 0
    var msg: string = "hello"
    static.pl>
    display{msg}
.>

```

This is a **breaking change** for existing code that relies on manual `free{}` calls, but all existing code will continue to work.

### if/else Statements

Full conditional branching is now supported:

```de
if x == 5 {
    display{msg}
} else {
    display{err}
}
```

### Struct Types

Define custom data structures with named fields:

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

### Field Access

Access struct fields using dot notation:

```de
player.health = (player.health - 1)
player.score = (player.score + 100)
```

---

## 📦 Installation

### Windows

Download and run the installer:
- **defacto-0.30-installer.exe**

The installer includes:
- Defacto compiler
- Example files
- PATH configuration

**Requirements:** NASM must be installed first.

### macOS / Linux

```bash
git clone https:
cd Defacto/compiler
make
./defacto -h
```

---

## 🔧 Technical Changes

### Compiler

| File | Changes |
| --- | --- |
| `defacto.h` | Added `ELSE`, `STRUCT`, `DOT` tokens; Added `StructDecl`, `StructFieldAccess` nodes |
| `lexer.h` | Added `else`, `struct` keywords; Added `.` token |
| `parser.h` | Added `parse_struct()`; Updated `parse_stmt()` for else and field access |
| `codegen.h` | Added struct layout; Added else branch generation; Auto-free implementation |

### Memory Model

- Variables are now automatically freed at the end of each section
- `free{}` is still supported for backward compatibility
- Struct fields are stored inline (no pointers)

---

## 📚 Documentation

New comprehensive syntax documentation: `SYNTAX.md`

Includes:
- Complete language reference
- All supported types
- Expression rules
- Statement syntax
- Examples

---

## 🐛 Bug Fixes

- Fixed driver call code generation (`call #keyboard_driver`)
- Fixed memory leak detection (now auto-frees)
- Fixed parser for struct types in variable declarations

---

## ⚠️ Breaking Changes

### Windows Installation

- Removed MSYS2 build instructions
- Installer is now the only supported method for Windows

### Memory Management

- `free{}` is no longer required
- Existing code with `free{}` will continue to work

---

## 📝 Example Code

### Complete Program with All New Features

```de
#Mainprogramm.start
#NO_RUNTIME
#SAFE


struct Player {
    x: i32
    y: i32
    health: i32
    score: i32
}


<drv.
Const.driver = keyboard_driver
keyboard_driver <<func = keyboard>>
.dr>


<.de
    var player: Player
    var msg: string = "Game Start!"
    var counter: i32 = 10

    static.pl>


    display{msg}


    if counter == 10 {
        player.score = 100
    } else {
        player.score = 0
    }


    player.x = 50
    player.y = 100
    player.health = 3
    player.score = (player.score + 50)


    loop {
        if counter == 0 { stop }
        counter = (counter - 1)
    }


    call #keyboard_driver
.>

#Mainprogramm.end
```

---

## 🚀 What's Next?

Planned for v0.31:
- More comparison operators (`!=`, `<`, `>`)
- Nested expressions
- Negative number literals
- Interrupt handling

---

## 📊 Statistics

| Metric | Value |
| --- | --- |
| Lines of code changed | ~500+ |
| New tokens | 3 |
| New AST nodes | 2 |
| New features | 4 |

---

## 🙏 Credits

Developed by the Defacto Team

**License:** See repository for license information

---

## 📥 Download

- **Windows Installer:** [defacto-0.30-installer.exe](https:
- **Source Code:** [GitHub Releases](https:

---

## 🔗 Links

- [Full Documentation (SYNTAX.md)](SYNTAX.md)
- [README.md](README.md)
- [README.ru.md](README.ru.md)
- [GitHub Repository](https:
