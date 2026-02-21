//! Defacto Backend Framework
//! 
//! A simple HTTP web framework for Defacto language written in Rust.
//! Supports routing, JSON responses, and request handling.
//!
//! # Example
//! ```rust
//! use defacto_backend::*;
//! 
//! // Initialize server
//! server_init(8080);
//! 
//! // Add route
//! route_get("/hello", hello_handler);
//! 
//! // Start server
//! server_start();
//! ```

use std::collections::HashMap;
use std::ffi::CString;
use std::os::raw::c_char;

use http_body_util::Full;
use hyper::body::Bytes;
use hyper::server::conn::http1;
use hyper::service::service_fn;
use hyper::{Method, Request, Response, StatusCode};
use hyper_util::rt::TokioIo;
use serde_json::json;
use tokio::net::TcpListener;

// ============================================
// Types
// ============================================

type HandlerFn = fn(&Request<hyper::body::Incoming>) -> Response<Full<Bytes>>;
type RouteMap = HashMap<String, HandlerFn>;

// Global state
static mut SERVER_HANDLE: Option<tokio::runtime::Runtime> = None;
static mut ROUTES_GET: Option<RouteMap> = None;
static mut ROUTES_POST: Option<RouteMap> = None;
static mut SERVER_PORT: u16 = 8080;

// ============================================
// Defacto Addon Interface
// ============================================

/// Required: Addon initialization
#[no_mangle]
pub extern "C" fn defo_addon_init(_api: *mut std::ffi::c_void) -> i32 {
    unsafe {
        ROUTES_GET = Some(HashMap::new());
        ROUTES_POST = Some(HashMap::new());
    }
    0
}

/// Required: Addon name
#[no_mangle]
pub extern "C" fn defo_addon_name() -> *const c_char {
    CString::new("defacto.backend")
        .expect("Failed to create addon name")
        .into_raw()
}

// ============================================
// Server API
// ============================================

/// Initialize HTTP server on a port
/// Returns 0 on success
#[no_mangle]
pub extern "C" fn server_init(port: i32) -> i32 {
    unsafe {
        SERVER_PORT = port as u16;
        
        if ROUTES_GET.is_none() {
            ROUTES_GET = Some(HashMap::new());
        }
        if ROUTES_POST.is_none() {
            ROUTES_POST = Some(HashMap::new());
        }
    }
    0
}

/// Add a GET route
/// path: URL path (e.g., "/hello")
/// handler: function pointer to handler
#[no_mangle]
pub extern "C" fn route_get(path: *const c_char, _handler: usize) -> i32 {
    unsafe {
        if ROUTES_GET.is_none() {
            ROUTES_GET = Some(HashMap::new());
        }
        
        if let Some(ref mut routes) = ROUTES_GET {
            if let Ok(path_str) = CString::from_raw(path as *mut c_char).into_string() {
                // Note: Handler registration needs special handling
                // For now, we use a simple approach
                routes.insert(path_str, handle_default);
                return 0;
            }
        }
    }
    1
}

/// Add a POST route
#[no_mangle]
pub extern "C" fn route_post(path: *const c_char, _handler: usize) -> i32 {
    unsafe {
        if ROUTES_POST.is_none() {
            ROUTES_POST = Some(HashMap::new());
        }
        
        if let Some(ref mut routes) = ROUTES_POST {
            if let Ok(path_str) = CString::from_raw(path as *mut c_char).into_string() {
                routes.insert(path_str, handle_default);
                return 0;
            }
        }
    }
    1
}

/// Start the HTTP server (blocking)
/// Call this at the end of your program
#[no_mangle]
pub extern "C" fn server_start() -> i32 {
    let port = unsafe { SERVER_PORT };
    
    println!("🚀 Starting Defacto backend server on port {}", port);
    
    let rt = match tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
    {
        Ok(rt) => rt,
        Err(e) => {
            eprintln!("Failed to create runtime: {}", e);
            return 1;
        }
    };

    rt.block_on(async {
        use std::net::SocketAddr;
        
        let addr: SocketAddr = ([127, 0, 0, 1], port).into();
        
        let listener = match TcpListener::bind(addr).await {
            Ok(l) => l,
            Err(e) => {
                eprintln!("Failed to bind to port {}: {}", port, e);
                return 1;
            }
        };

        println!("✅ Server listening on http://{}", addr);

        loop {
            match listener.accept().await {
                Ok((stream, _)) => {
                    let io = TokioIo::new(stream);
                    
                    tokio::task::spawn(async move {
                        if let Err(err) = http1::Builder::new()
                            .serve_connection(io, service_fn(handle_request))
                            .await
                        {
                            eprintln!("Error serving connection: {:?}", err);
                        }
                    });
                }
                Err(e) => {
                    eprintln!("Error accepting connection: {}", e);
                }
            }
        }
    });

    0
}

// ============================================
// Internal Handlers
// ============================================

fn handle_default(_req: &Request<hyper::body::Incoming>) -> Response<Full<Bytes>> {
    Response::builder()
        .status(StatusCode::OK)
        .header("Content-Type", "application/json")
        .body(Full::new(Bytes::from(
            serde_json::to_string(&json!({
                "message": "Hello from Defacto Backend!",
                "status": "ok"
            })).unwrap()
        )))
        .unwrap()
}

async fn handle_request(
    req: Request<hyper::body::Incoming>,
) -> Result<Response<Full<Bytes>>, hyper::Error> {
    let path = req.uri().path().to_string();
    let method = req.method();
    
    println!("📩 Request: {} {}", method, path);

    let response = unsafe {
        match method {
            &Method::GET => {
                if let Some(ref routes) = ROUTES_GET {
                    if let Some(handler) = routes.get(&path) {
                        handler(&req)
                    } else if path == "/" {
                        handle_default(&req)
                    } else {
                        not_found()
                    }
                } else {
                    handle_default(&req)
                }
            }
            &Method::POST => {
                if let Some(ref routes) = ROUTES_POST {
                    if let Some(handler) = routes.get(&path) {
                        handler(&req)
                    } else {
                        not_found()
                    }
                } else {
                    handle_default(&req)
                }
            }
            _ => {
                method_not_allowed()
            }
        }
    };

    Ok(response)
}

fn not_found() -> Response<Full<Bytes>> {
    Response::builder()
        .status(StatusCode::NOT_FOUND)
        .header("Content-Type", "application/json")
        .body(Full::new(Bytes::from(
            serde_json::to_string(&json!({
                "error": "Not Found",
                "status": 404
            })).unwrap()
        )))
        .unwrap()
}

fn method_not_allowed() -> Response<Full<Bytes>> {
    Response::builder()
        .status(StatusCode::METHOD_NOT_ALLOWED)
        .header("Content-Type", "application/json")
        .body(Full::new(Bytes::from(
            serde_json::to_string(&json!({
                "error": "Method Not Allowed",
                "status": 405
            })).unwrap()
        )))
        .unwrap()
}

// ============================================
// JSON Helper Functions
// ============================================

/// Create a JSON response string
/// Returns allocated string that must be freed with json_free
#[no_mangle]
pub extern "C" fn json_object() -> *mut c_char {
    CString::new("{}")
        .unwrap()
        .into_raw()
}

/// Free a JSON string allocated by this library
#[no_mangle]
pub unsafe extern "C" fn json_free(s: *mut c_char) {
    if !s.is_null() {
        let _ = CString::from_raw(s);
    }
}

/// Get version of the backend framework
#[no_mangle]
pub extern "C" fn backend_version() -> *const c_char {
    CString::new("0.1.0")
        .unwrap()
        .into_raw()
}
