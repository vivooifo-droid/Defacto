# Defacto Rust Addons

Write Defacto language libraries and addons in Rust!

## Quick Start

### Requirements

- [Rust](https:
- Cargo (comes with Rust)

### Build

```bash
cd addons/rust
make
```

Or directly with Cargo:

```bash
cargo build --release
```

Output will be in `target/release/libsample_addon.(so|dylib|dll)`

### Test

```bash
make test      # Run Rust unit tests
make run       # Load addon with runtime loader
```

## Creating Your Own Addon

### 1. Initialize a new Rust library

```bash
cargo new --lib my_addon
cd my_addon
```

### 2. Edit `Cargo.toml`

```toml
[package]
name = "my_addon"
version = "0.1.0"
edition = "2021"

[lib]
name = "my_addon"
crate-type = ["cdylib"]  # Important: creates a dynamic library

[dependencies]
```

### 3. Write your addon (`src/lib.rs`)

```rust
use std::ffi::CString;
use std::os::raw::c_char;


#[no_mangle]
pub extern "C" fn defo_addon_init(_api: *mut std::ffi::c_void) -> i32 {

    0
}


#[no_mangle]
pub extern "C" fn defo_addon_name() -> *const c_char {
    CString::new("my_addon")
        .expect("Failed to create name")
        .into_raw()
}


#[no_mangle]
pub extern "C" fn my_function(a: i32, b: i32) -> i32 {
    a + b
}
```

### 4. Build

```bash
cargo build --release
```

### 5. Use in Defacto

```de
<.de

    #LOAD {my_addon}
    

    var result: i32 = 0
    result = my_function(5, 3)
    
    display{result}
.>
```

## API Reference

### Required Functions

Every Defacto addon must implement these two functions:

#### `defo_addon_init`

```rust
#[no_mangle]
pub extern "C" fn defo_addon_init(_api: *mut std::ffi::c_void) -> i32 {

    0
}
```

Called when the addon is loaded. Return 0 on success.

#### `defo_addon_name`

```rust
#[no_mangle]
pub extern "C" fn defo_addon_name() -> *const c_char {
    CString::new("my_addon")
        .unwrap()
        .into_raw()
}
```

Returns the addon name as a null-terminated C string.

### Exporting Functions

Use `#[no_mangle]` and `extern "C"` to make functions callable from Defacto:

```rust
#[no_mangle]
pub extern "C" fn my_func(x: i32) -> i32 {
    x * 2
}
```

### Working with Strings

```rust
use std::ffi::{CStr, CString};
use std::os::raw::c_char;


#[no_mangle]
pub unsafe extern "C" fn process_string(s: *const c_char) -> i32 {
    if s.is_null() {
        return 0;
    }
    let rust_str = CStr::from_ptr(s);

    1
}


#[no_mangle]
pub extern "C" fn get_message() -> *mut c_char {
    CString::new("Hello from Rust!")
        .unwrap()
        .into_raw()
}


#[no_mangle]
pub unsafe extern "C" fn free_string(s: *mut c_char) {
    if !s.is_null() {
        let _ = CString::from_raw(s);
    }
}
```

## Example Functions

See `src/lib.rs` for complete examples:

- `rust_add(a, b)` - Add two numbers
- `rust_mul(a, b)` - Multiply two numbers
- `rust_is_even(n)` - Check if even
- `rust_strlen(s)` - Get string length
- `rust_to_upper(s)` - Convert to uppercase
- `rust_version()` - Get addon version

## Platform Support

| Platform | Extension | Command |
|----------|-----------|---------|
| Linux | `.so` | `make` |
| macOS | `.dylib` | `make` |
| Windows | `.dll` | `cargo build --release` |

## Tips

1. **Always use `#[no_mangle]`** - Prevents Rust from mangling function names
2. **Use `extern "C"`** - Ensures C ABI compatibility
3. **Handle null pointers** - Always check pointers from external code
4. **Memory management** - Document who owns returned allocations
5. **Test thoroughly** - Use `cargo test` for unit tests

## License

MIT
