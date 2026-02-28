#include "src/defacto.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/codegen.h"
#include "src/arm64_codegen.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <iostream>

static std::string read_file(const std::string& p){
    std::ifstream f(p);
    if(!f) throw std::runtime_error("cannot open '"+p+"'");
    std::ostringstream s; s<<f.rdbuf(); return s.str();
}

static std::string sh_quote(const std::string& s){
    std::string out="'";
    for(char c: s){
        if(c=='\'') out += "'\\''";
        else out += c;
    }
    out += "'";
    return out;
}

static void usage(const char* prog){
    std::cout
        <<"Defacto Compiler v0.49\n\n"
        <<"Usage: "<<prog<<" [options] <file.de>\n\n"
        <<"Options:\n"
        <<"  -o <file>       output file (default: a.out)\n"
        <<"  -S              emit assembly only, do not assemble\n"
        <<"  -kernel         bare-metal mode: [BITS 32][ORG 0x1000] + hlt (default)\n"
        <<"  -terminal       terminal mode: Linux 32-bit syscalls\n"
        <<"  -terminal64     terminal mode: Linux 64-bit syscalls\n"
        <<"  -terminal-macos terminal mode: macOS x86_64 syscalls\n"
        <<"  -terminal-arm64 terminal mode: macOS/Linux ARM64 syscalls\n"
        <<"  -v              verbose\n"
        <<"  -h              help\n\n"
        <<"Examples:\n"
        <<"  "<<prog<<" -terminal hello.de       # Linux 32-bit\n"
        <<"  "<<prog<<" -terminal64 hello.de     # Linux 64-bit\n"
        <<"  "<<prog<<" -terminal-macos app.de   # macOS x86_64\n"
        <<"  "<<prog<<" -terminal-arm64 app.de   # macOS/ARM64, Linux/ARM64\n"
        <<"  "<<prog<<" -kernel -o kernel.bin os.de\n";
}

int main(int argc, char** argv){
    if(argc<2){usage(argv[0]);return 1;}

    std::string input, output="a.out";
    bool asm_only=false, verbose=false;
    bool bare_metal=true, macos_terminal=false, linux64_terminal=false, arm64_terminal=false, macos_arm64=false;

    // Auto-detect platform
    #ifdef __APPLE__
    bare_metal = false;
    #if defined(__arm64__) || defined(__aarch64__)
    arm64_terminal = true;
    macos_arm64 = true;
    #else
    macos_terminal = true;
    #endif
    #endif

    for(int i=1;i<argc;i++){
        const std::string a=argv[i];
        if(a=="-h"){usage(argv[0]);return 0;}
        else if(a=="-S")        asm_only=true;
        else if(a=="-v")        verbose=true;
        else if(a=="-kernel")   { bare_metal=true; macos_terminal=false; linux64_terminal=false; arm64_terminal=false; }
        else if(a=="-terminal") { bare_metal=false; macos_terminal=false; linux64_terminal=false; arm64_terminal=false; }
        else if(a=="-terminal64") { bare_metal=false; macos_terminal=false; linux64_terminal=true; arm64_terminal=false; }
        else if(a=="-terminal-macos") { bare_metal=false; macos_terminal=true; linux64_terminal=false; arm64_terminal=false; }
        else if(a=="-terminal-arm64") { bare_metal=false; macos_terminal=false; linux64_terminal=false; arm64_terminal=true; }
        else if(a=="-o"){if(++i>=argc){err("'-o' requires filename");return 1;} output=argv[i];}
        else if(a[0]!='-') input=a;
        else{err("unknown option '"+a+"'");return 1;}
    }
    if(input.empty()){err("no input file");return 1;}

    std::string stem=input;
    const auto dot=stem.find_last_of('.');
    if(dot!=std::string::npos) stem=stem.substr(0,dot);
    std::string asm_file=stem+".asm";

    try{
        if(verbose) std::cout<<"reading "<<input<<"\n";
        std::string src = read_file(input);
        
        // Find imports in source using simple regex-like parsing
        std::vector<std::string> imports;
        size_t pos = 0;
        while((pos = src.find("Import{", pos)) != std::string::npos) {
            size_t start = pos + 7;  // length of "Import{"
            size_t end = src.find("}", start);
            if(end != std::string::npos) {
                std::string libname = src.substr(start, end - start);
                imports.push_back(libname);
                if(verbose) std::cout<<"  found import: " << libname << "\n";
            }
            pos = end + 1;
        }
        
        // Load imported libraries and insert AFTER directives
        std::string full_src = "";
        bool inserted = false;
        
        // Read main file line by line
        std::istringstream iss(src);
        std::string line;
        bool past_imports = false;
        while(std::getline(iss, line)) {
            // Skip Import{} lines (already processed)
            if(line.find("Import{") != std::string::npos) {
                past_imports = true;
                continue;  // Skip this line
            }
            full_src += line + "\n";
            // Insert libraries after all directives and Import statements
            if(!inserted && past_imports && 
                line.find("#Mainprogramm.start") == std::string::npos &&
                line.find("#NO_RUNTIME") == std::string::npos &&
                line.find("#SAFE") == std::string::npos &&
                line.find("#DRIVER") == std::string::npos) {
                // This line is after all directives, insert here
                for(const auto& lib : imports) {
                    if(verbose) std::cout<<"  importing: " << lib << ".de\n";
                    std::string lib_path = lib + ".de";
                    std::string lib_src;
                    try {
                        lib_src = read_file(lib_path);
                    } catch(...) {
                        size_t last_slash = input.find_last_of('/');
                        if(last_slash != std::string::npos) {
                            std::string base_dir = input.substr(0, last_slash + 1);
                            lib_src = read_file(base_dir + lib_path);
                        } else {
                            try {
                                lib_src = read_file("lib/" + lib_path);
                            } catch(...) {
                                err("library not found: " + lib);
                                return 1;
                            }
                        }
                    }
                    full_src += lib_src + "\n";
                }
                inserted = true;
            }
        }

        if(verbose) std::cout<<"parsing...\n";
        Lexer  lexer(full_src);
        Parser parser(lexer.tokenize());
        auto ast=parser.parse(false);

        if(verbose){
            std::cout<<"  no_runtime: "<<ast->no_runtime<<"\n";
            std::cout<<"  functions:  "<<ast->functions.size()<<"\n";
            std::cout<<"  mode: "<<(bare_metal?"kernel (bare-metal)":(arm64_terminal?"ARM64 terminal":(linux64_terminal?"Linux 64-bit":"Linux 32-bit")) )<<"\n";
        }

        if (arm64_terminal) {
            // Use ARM64 codegen
            ARM64CodeGen cg;
            cg.set_mode(macos_arm64);
            cg.emit(ast.get(), asm_file);
        } else {
            // Use x86 codegen
            CodeGen cg;
            cg.set_mode(bare_metal, macos_terminal, linux64_terminal, arm64_terminal);
            cg.emit(ast.get(), asm_file);
        }

        if(asm_only){std::cout<<"done: "<<asm_file<<"\n";return 0;}

        if(bare_metal){
            const std::string cmd="nasm -f bin "+sh_quote(asm_file)+" -o "+sh_quote(output);
            if(verbose) std::cout<<"$ "<<cmd<<"\n";
            if(std::system(cmd.c_str())!=0){err("assembler failed");return 1;}
        } else if(arm64_terminal) {
            // ARM64 (macOS or Linux) - uses ARM64 assembly directly
            const std::string obj=stem+".o";
            #ifdef __APPLE__
            const std::string cmd_nasm="as -arch arm64 -o "+sh_quote(obj)+" "+sh_quote(asm_file);
            const char* cc_env = std::getenv("DEFACTO_CC");
            const std::string cc_bin = cc_env ? cc_env : "clang";
            const std::string cmd_ld  = cc_bin+" -arch arm64 -o "+sh_quote(output)+" "+sh_quote(obj);
            #else
            const std::string cmd_nasm="as -o "+sh_quote(obj)+" "+sh_quote(asm_file);
            const char* ld_env = std::getenv("DEFACTO_LD");
            const std::string ld_bin = ld_env ? ld_env : "ld";
            const std::string cmd_ld  = ld_bin+" -m aarch64linux -o "+sh_quote(output)+" "+sh_quote(obj)+" -lc";
            #endif
            if(verbose) std::cout<<"$ "<<cmd_nasm<<"\n$ "<<cmd_ld<<"\n";
            if(std::system(cmd_nasm.c_str())!=0){err("assembler failed");return 1;}
            if(std::system(cmd_ld.c_str())!=0){err("linker failed");return 1;}
            if(!verbose) std::remove(obj.c_str());
        } else if(linux64_terminal) {
            // Linux 64-bit ELF
            const std::string obj=stem+".o";
            const std::string cmd_nasm="nasm -f elf64 "+sh_quote(asm_file)+" -o "+sh_quote(obj);
            const char* ld_env = std::getenv("DEFACTO_LD");
            const std::string ld_bin = ld_env ? ld_env : "ld";
            const std::string cmd_ld  = ld_bin+" -m elf_x86_64 -o "+sh_quote(output)+" "+sh_quote(obj)+" -lc";
            if(verbose) std::cout<<"$ "<<cmd_nasm<<"\n$ "<<cmd_ld<<"\n";
            if(std::system(cmd_nasm.c_str())!=0){err("assembler failed");return 1;}
            if(std::system(cmd_ld.c_str())!=0){err("linker failed");return 1;}
            if(!verbose) std::remove(obj.c_str());
        } else if(!macos_terminal) {
            const std::string obj=stem+".o";
            const std::string cmd_nasm="nasm -f elf32 "+sh_quote(asm_file)+" -o "+sh_quote(obj);
            // Link with libc for malloc/free support
            // On macOS, use clang with -m32; on Linux use ld with -m elf_i386
            #ifdef __APPLE__
            const char* cc_env = std::getenv("DEFACTO_CC");
            const std::string cc_bin = cc_env ? cc_env : "clang";
            const std::string cmd_ld  = cc_bin+" -m32 -o "+sh_quote(output)+" "+sh_quote(obj);
            std::cout<<"warning: -terminal mode (32-bit Linux) is not fully supported on macOS.\n";
            std::cout<<"         Consider using -terminal-macos for native macOS binaries.\n";
            #else
            const char* ld_env = std::getenv("DEFACTO_LD");
            const std::string ld_bin = ld_env ? ld_env : "ld";
            const std::string cmd_ld  = ld_bin+" -m elf_i386 -o "+sh_quote(output)+" "+sh_quote(obj)+" -lc";
            #endif
            if(verbose) std::cout<<"$ "<<cmd_nasm<<"\n$ "<<cmd_ld<<"\n";
            if(std::system(cmd_nasm.c_str())!=0){err("assembler failed");return 1;}
            if(std::system(cmd_ld.c_str())!=0){err("linker failed");return 1;}
            if(!verbose) std::remove(obj.c_str());
        } else {
            const std::string obj=stem+".o";
            const std::string cmd_nasm="nasm -f macho64 "+sh_quote(asm_file)+" -o "+sh_quote(obj);
            const char* cc_env = std::getenv("DEFACTO_CC");
            const std::string cc_bin = cc_env ? cc_env : "clang";
            const std::string cmd_ld  = cc_bin+" -arch x86_64 -Wl,-e,_start -Wl,-platform_version,macos,11.0,11.0 -o "+sh_quote(output)+" "+sh_quote(obj);
            if(verbose) std::cout<<"$ "<<cmd_nasm<<"\n$ "<<cmd_ld<<"\n";
            if(std::system(cmd_nasm.c_str())!=0){err("assembler failed");return 1;}
            if(std::system(cmd_ld.c_str())!=0){err("linker failed");return 1;}
            if(!verbose) std::remove(obj.c_str());
        }

        if(!verbose) std::remove(asm_file.c_str());
        std::cout<<"done: "<<output<<"\n";

    }catch(const std::exception& e){err(e.what());return 1;}
    return 0;
}