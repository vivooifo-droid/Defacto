# Defacto v0.40 - MEGA RELEASE

**Release Date:** February 28, 2026

## 🎉 What's New

### Comparison Operators (5 new!)
- `!=` - Not equal
- `<` - Less than
- `>` - Greater than
- `<=` - Less than or equal
- `>=` - Greater than or equal

### Return from Functions
```de
function == add {
    <.de
        var a: i32 = 0
        var b: i32 = 0
        static.pl>
        return{(a + b)}
    .>
}
```

### While Loops
```de
var i: i32 = 1
while i <= 5 {
    printnum{i}
    i = (i + 1)
}
```

### Library Imports
```de
Import{math_lib}
call #square
```

### Homebrew Support
```bash
brew tap vivooifo-droid/Defacto
brew install defacto
```

## 📦 Installation

### macOS (Homebrew)
```bash
brew tap vivooifo-droid/Defacto
brew install defacto
```

### Linux/macOS (Manual)
```bash
cd compiler && make
./defacto -h
```

## 🧪 Testing

```bash
# Comparison operators
./compiler/defacto test-compare.de -o test-compare
./test-compare

# While loops
./compiler/defacto test-while.de -o test-while
./test-while

# Return from functions
./compiler/defacto test-return3.de -o test-return3
./test-return3

# Library imports
./compiler/defacto test-import.de -o test-import
./test-import
```

## 📝 Changelog

### v0.40 (2026-02-28)
- ✨ Comparison operators: `!=`, `<`, `>`, `<=`, `>=`
- ✨ Return from functions: `return{value}`
- ✨ While loops: `while condition { ... }`
- ✨ Library imports: `Import{name}`
- ✨ Homebrew installation support
- 🐛 Fixed imul in 64-bit mode
- 🐛 Fixed label generation for nested expressions
- 📚 Updated documentation

### v0.35 (Previous)
- Pointers (`*i32`, `&x`, `*ptr`, `ptr->field`)
- System allocator (malloc/free)
- Null pointers
- Double pointers

## 🎯 Roadmap to v0.50

- [ ] For loops (full support)
- [ ] Arrays with initialization `[1, 2, 3]`
- [ ] String concatenation
- [ ] Block comments `/* */`
- [ ] Enums
- [ ] Standard library

## ⚠️ Known Limitations

1. **for loops** - require simple step value (no expressions in parentheses)
2. **Strings** - don't support spaces (only `\n` and `\t`)
3. **Libraries** - require unique variable names

## 📊 Statistics

- **Files changed:** 20+
- **Lines of code added:** 800+
- **New keywords:** 7 (`return`, `while`, `for`, `enum`, `try`, `catch`, `;`)
- **New operators:** 5 (`!=`, `<`, `>`, `<=`, `>=`)
- **New constructs:** 2 (`while`, `for`)

## 🙏 Credits

Thanks to everyone using Defacto! 🎉

---

**Defacto v0.40 - The Biggest Release Ever!** 🚀
