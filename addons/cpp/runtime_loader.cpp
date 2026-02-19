#include <iostream>
#include <string>

#if defined(_WIN32)
#include <windows.h>
using LibHandle = HMODULE;
static LibHandle load_library(const char* path) { return LoadLibraryA(path); }
static void* load_symbol(LibHandle h, const char* name) { return reinterpret_cast<void*>(GetProcAddress(h, name)); }
static void close_library(LibHandle h) { if (h) FreeLibrary(h); }
static std::string last_error() {
    DWORD code = GetLastError();
    return "win32 error code=" + std::to_string(static_cast<unsigned long>(code));
}
#else
#include <dlfcn.h>
using LibHandle = void*;
static LibHandle load_library(const char* path) { return dlopen(path, RTLD_NOW); }
static void* load_symbol(LibHandle h, const char* name) { return dlsym(h, name); }
static void close_library(LibHandle h) { if (h) dlclose(h); }
static std::string last_error() {
    const char* err = dlerror();
    return err ? std::string(err) : std::string("unknown dlerror");
}
#endif

using AddonInitFn = int(*)(void* api);
using AddonNameFn = const char*(*)();

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: runtime_loader <addon.(so|dylib|dll)>\n";
        return 1;
    }

    const char* path = argv[1];
    LibHandle lib = load_library(path);
    if (!lib) {
        std::cerr << "error: failed to load addon: " << path << "\n";
        std::cerr << "detail: " << last_error() << "\n";
        return 1;
    }

    auto init_fn = reinterpret_cast<AddonInitFn>(load_symbol(lib, "defo_addon_init"));
    auto name_fn = reinterpret_cast<AddonNameFn>(load_symbol(lib, "defo_addon_name"));

    if (!init_fn || !name_fn) {
        std::cerr << "error: required symbols not found (defo_addon_init / defo_addon_name)\n";
        close_library(lib);
        return 1;
    }

    const char* addon_name = name_fn();
    int rc = init_fn(nullptr);

    std::cout << "addon: " << (addon_name ? addon_name : "<null>") << "\n";
    std::cout << "init: " << rc << "\n";

    close_library(lib);
    return rc == 0 ? 0 : 2;
}
