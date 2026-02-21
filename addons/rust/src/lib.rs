//! Defacto Rust Addon - Sample Library
//! 
//! This is a sample addon showing how to write Defacto libraries in Rust.
//! 
//! Build: cargo build --release
//! Output: target/release/libsample_addon.(so|dylib|dll)

use std::ffi::CString;
use std::os::raw::c_char;

/// Addon initialization function
/// Called when the addon is loaded by Defacto runtime
/// 
/// # Safety
/// The api pointer is currently unused and may be null
#[no_mangle]
pub extern "C" fn defo_addon_init(_api: *mut std::ffi::c_void) -> i32 {
    // Initialize your addon here
    // Return 0 on success, non-zero on error
    0
}

/// Addon name function
/// Returns the name of the addon as a C string
/// 
/// # Safety
/// Returns a static C string, safe to call from C
#[no_mangle]
pub extern "C" fn defo_addon_name() -> *const c_char {
    CString::new("sample.rust_addon")
        .expect("Failed to create addon name")
        .into_raw()
}

// ============================================
// Example functions that can be called from Defacto
// ============================================

/// Add two integers
/// Example: result = rust_add(5, 3)  // returns 8
#[no_mangle]
pub extern "C" fn rust_add(a: i32, b: i32) -> i32 {
    a + b
}

/// Multiply two integers
/// Example: result = rust_mul(4, 7)  // returns 28
#[no_mangle]
pub extern "C" fn rust_mul(a: i32, b: i32) -> i32 {
    a * b
}

/// Check if a number is even
/// Example: is_even = rust_is_even(10)  // returns 1 (true)
#[no_mangle]
pub extern "C" fn rust_is_even(n: i32) -> i32 {
    if n % 2 == 0 { 1 } else { 0 }
}

/// Get the length of a string (null-terminated)
/// 
/// # Safety
/// This function assumes the pointer points to a valid null-terminated C string
#[no_mangle]
pub unsafe extern "C" fn rust_strlen(s: *const c_char) -> i32 {
    if s.is_null() {
        return 0;
    }
    std::ffi::CStr::from_ptr(s)
        .to_bytes()
        .len() as i32
}

/// Convert a string to uppercase (returns new allocated string)
/// Note: Caller is responsible for freeing the returned string
/// 
/// # Safety
/// This function assumes the pointer points to a valid null-terminated C string
#[no_mangle]
pub unsafe extern "C" fn rust_to_upper(s: *const c_char) -> *mut c_char {
    if s.is_null() {
        return std::ptr::null_mut();
    }
    
    let cstr = std::ffi::CStr::from_ptr(s);
    let upper: String = cstr
        .to_string_lossy()
        .chars()
        .map(|c| c.to_ascii_uppercase())
        .collect();
    
    CString::new(upper)
        .unwrap()
        .into_raw()
}

/// Free a string allocated by rust_to_upper
/// 
/// # Safety
/// Only call this on pointers returned by rust_to_upper
#[no_mangle]
pub unsafe extern "C" fn rust_free_string(s: *mut c_char) {
    if !s.is_null() {
        let _ = CString::from_raw(s);
    }
}

/// Get addon version
/// Returns version as "major.minor.patch" format
#[no_mangle]
pub extern "C" fn rust_version() -> *const c_char {
    CString::new("0.1.0")
        .expect("Failed to create version string")
        .into_raw()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add() {
        assert_eq!(rust_add(2, 3), 5);
        assert_eq!(rust_add(-1, 1), 0);
    }

    #[test]
    fn test_mul() {
        assert_eq!(rust_mul(4, 5), 20);
    }

    #[test]
    fn test_is_even() {
        assert_eq!(rust_is_even(4), 1);
        assert_eq!(rust_is_even(7), 0);
    }
}
