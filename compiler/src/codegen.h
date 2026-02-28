#pragma once
#include "defacto.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

class CodeGen {
    std::ostringstream code;
    std::ostringstream data;
    std::ostringstream externs;  // For extern declarations (malloc, free)

    std::map<std::string,std::string> var_lbl;
    std::map<std::string,bool> var_is_ptr;
    std::map<std::string,bool> var_on_heap;  // true if variable allocated on heap
    std::map<std::string,std::string> var_type;  // variable name -> type name
    std::map<std::string, std::map<std::string, int>> struct_field_offsets;  // struct_type -> (field_name -> offset)
    std::map<std::string, int> struct_sizes;  // struct_type -> total size in bytes
    std::set<std::string> declared, freed, const_declared, driver_constants;
    std::vector<std::string> loop_ends;
    int lcnt = 0, scnt = 0;
    bool bare_metal = true;
    bool macos_terminal = false;
    bool use_allocator = false;  // Use system allocator (malloc/free)

    std::string lbl(const std::string& pfx="L") { return pfx+std::to_string(lcnt++); }
    std::string addr(const std::string& sym) { return macos_terminal ? ("rel "+sym) : sym; }

    std::string reg(const std::string& r) {
        static const std::map<std::string,std::string> m32 = {
            {"#R1","edi"}, {"#R2","esi"}, {"#R3","edx"}, {"#R4","ecx"},
            {"#R5","ebx"}, {"#R6","eax"}, {"#R7","edi"}, {"#R8","esi"},
            {"#R9","ebx"}, {"#R10","ecx"},{"#R11","edx"},{"#R12","esi"},
            {"#R13","edi"},{"#R14","eax"},{"#R15","ebp"},{"#R16","esp"}
        };
        static const std::map<std::string,std::string> m64 = {
            {"#R1","rdi"}, {"#R2","rsi"}, {"#R3","rdx"}, {"#R4","rcx"},
            {"#R5","rbx"}, {"#R6","rax"}, {"#R7","rdi"}, {"#R8","rsi"},
            {"#R9","rbx"}, {"#R10","rcx"},{"#R11","rdx"},{"#R12","rsi"},
            {"#R13","rdi"},{"#R14","rax"},{"#R15","rbp"},{"#R16","rsp"}
        };
        const auto& m = macos_terminal ? m64 : m32;
        auto it=m.find(r); return it!=m.end()?it->second:"eax";
    }

    bool is_reg(const std::string& s){ return s.size()>=3&&s[0]=='#'&&s[1]=='R'&&isdigit(s[2]); }
    bool is_num(const std::string& s){ return !s.empty()&&(isdigit(s[0])||(s[0]=='-'&&s.size()>1&&isdigit(s[1]))); }
    bool is_hex(const std::string& s){ return s.size()>2&&s[0]=='0'&&(s[1]=='x'||s[1]=='X'); }
    bool parse_arr_ref(const std::string& s, std::string& name, std::string& idx){
        auto lb=s.find('[');
        auto rb=s.find(']');
        if(lb==std::string::npos||rb==std::string::npos||rb<=lb+1) return false;
        name=s.substr(0,lb);
        idx=s.substr(lb+1,rb-lb-1);
        return !name.empty() && !idx.empty();
    }

    void load(const std::string& dst, const std::string& src){
        if(is_reg(src))             code<<"    mov "<<dst<<", "<<reg(src)<<"\n";
        else if(is_num(src)||is_hex(src)) code<<"    mov "<<dst<<", "<<src<<"\n";
        else if(src.size() > 0 && src[0] == '&') {
            // Address-of: &var -> load address of var
            std::string varname = src.substr(1);
            auto it = var_lbl.find(varname);
            if(it == var_lbl.end()) throw std::runtime_error("undefined variable '"+varname+"'");
            code<<"    mov "<<dst<<", "<<addr(it->second)<<"\n";
        }
        else if(src.size() > 0 && src[0] == '*') {
            // Dereference: *ptr -> load value from pointer
            std::string ptrname = src.substr(1);
            auto it = var_lbl.find(ptrname);
            if(it == var_lbl.end()) throw std::runtime_error("undefined pointer '"+ptrname+"'");
            if(macos_terminal){
                // 64-bit: load 8-byte pointer
                code<<"    mov rcx, qword ["<<addr(it->second)<<"]\n";
                code<<"    mov "<<dst<<", dword [rcx]\n";
            } else {
                // 32-bit: load 4-byte pointer
                code<<"    mov ecx, dword ["<<addr(it->second)<<"]\n";
                code<<"    mov "<<dst<<", dword [ecx]\n";
            }
        }
        else if(src.size() > 0 && src[0] == '(') {
            // Expression in parentheses: evaluate it
            expr(dst, src);
        }
        else{
            std::string aname, aidx;
            if(parse_arr_ref(src,aname,aidx)){
                auto it=var_lbl.find(aname);
                if(it==var_lbl.end()) throw std::runtime_error("undefined array '"+aname+"'");
                if(is_reg(aidx)) code<<"    mov ecx, "<<reg(aidx)<<"\n";
                else if(is_num(aidx)) code<<"    mov ecx, "<<aidx<<"\n";
                else{
                    auto jt=var_lbl.find(aidx);
                    if(jt==var_lbl.end()) throw std::runtime_error("undefined variable '"+aidx+"'");
                    code<<"    mov ecx, dword ["<<addr(jt->second)<<"]\n";
                }
                code<<"    mov "<<dst<<", dword ["<<addr(it->second)<<" + ecx*4]\n";
                return;
            }
            auto it=var_lbl.find(src);
            if(it==var_lbl.end()) throw std::runtime_error("undefined variable '"+src+"'");
            if(macos_terminal && var_is_ptr[src]) code<<"    mov "<<dst<<", qword ["<<addr(it->second)<<"]\n";
            else code<<"    mov "<<dst<<", dword ["<<addr(it->second)<<"]\n";
        }
    }

    void store(const std::string& src_reg, const std::string& dst){
        if(const_declared.count(dst)) throw std::runtime_error("cannot assign to const '"+dst+"'");

        // Check for dereference: *ptr = value
        if(dst.size() > 0 && dst[0] == '*') {
            std::string ptrname = dst.substr(1);
            auto it = var_lbl.find(ptrname);
            if(it == var_lbl.end()) throw std::runtime_error("undefined pointer '"+ptrname+"'");
            if(macos_terminal){
                // 64-bit: load 8-byte pointer
                code<<"    mov rcx, qword ["<<addr(it->second)<<"]\n";
                code<<"    mov dword [rcx], "<<src_reg<<"\n";
            } else {
                // 32-bit: load 4-byte pointer
                code<<"    mov ecx, dword ["<<addr(it->second)<<"]\n";
                code<<"    mov dword [ecx], "<<src_reg<<"\n";
            }
            return;
        }

        auto it=var_lbl.find(dst);
        if(it==var_lbl.end()) throw std::runtime_error("undefined variable '"+dst+"'");
        if(macos_terminal && var_is_ptr[dst]) code<<"    mov qword ["<<addr(it->second)<<"], "<<src_reg<<"\n";
        else code<<"    mov dword ["<<addr(it->second)<<"], "<<src_reg<<"\n";
    }

    void expr(const std::string& dst, const std::string& e){
        // Parse and evaluate nested expressions
        // Format: ((a+b)*c) or (a+(b*c)) etc.
        std::string s = e;
        
        // Remove outer parentheses if present
        while (s.size() >= 2 && s[0] == '(' && s[s.size()-1] == ')') {
            // Check if these parens match each other
            int depth = 0;
            bool match = true;
            for (size_t i = 0; i < s.size(); i++) {
                if (s[i] == '(') depth++;
                else if (s[i] == ')') depth--;
                if (depth == 0 && i < s.size() - 1) {
                    match = false;
                    break;
                }
            }
            if (match) {
                s = s.substr(1, s.size() - 2);
            } else {
                break;
            }
        }
        
        // Find the main operator (lowest precedence first: + or -)
        // Scan from right to left to handle left-associative operators
        size_t op_pos = std::string::npos;
        char op_char = 0;
        int paren_depth = 0;
        
        // First pass: look for + or - (lowest precedence)
        for (int i = (int)s.size() - 1; i >= 0; i--) {
            char c = s[i];
            if (c == ')') paren_depth++;
            else if (c == '(') paren_depth--;
            else if (paren_depth == 0 && (c == '+' || c == '-')) {
                // Make sure it's not part of a negative number
                if (i == 0 || (s[i-1] != '(' && s[i-1] != '+' && s[i-1] != '-' && s[i-1] != '*' && s[i-1] != '/')) {
                    op_pos = i;
                    op_char = c;
                    break;
                }
            }
        }
        
        // Second pass: if no + or -, look for * or / (higher precedence)
        if (op_pos == std::string::npos) {
            paren_depth = 0;
            for (int i = (int)s.size() - 1; i >= 0; i--) {
                char c = s[i];
                if (c == ')') paren_depth++;
                else if (c == '(') paren_depth--;
                else if (paren_depth == 0 && (c == '*' || c == '/')) {
                    op_pos = i;
                    op_char = c;
                    break;
                }
            }
        }
        
        // No operator found: it's a value
        if (op_pos == std::string::npos) {
            load(dst, s);
            return;
        }
        
        // Split into left and right parts
        std::string left_str = s.substr(0, op_pos);
        std::string right_str = s.substr(op_pos + 1);
        char op = op_char;
        
        // Evaluate left side first
        expr(dst, left_str);
        
        // Check if right side is an expression
        if (right_str.size() > 0 && right_str[0] == '(') {
            // Evaluate right side into edx (not to conflict with dst)
            if (macos_terminal) {
                code << "    push r" << dst.substr(1) << "\n";  // Save left result (rax/rbx/etc)
            } else {
                code << "    push " << dst << "\n";  // Save left result
            }
            expr("edx", right_str);
            if (macos_terminal) {
                code << "    pop r" << dst.substr(1) << "\n";  // Restore left result
            } else {
                code << "    pop " << dst << "\n";  // Restore left result
            }
            // Apply operator
            if (op == '+') {
                code << "    add " << dst << ", edx\n";
            } else if (op == '-') {
                code << "    sub " << dst << ", edx\n";
            } else if (op == '*') {
                code << "    imul " << dst << ", edx\n";
            } else if (op == '/') {
                if (macos_terminal) {
                    code << "    push rdx\n";
                    code << "    xor edx, edx\n";
                    code << "    mov ecx, ecx\n";
                    if (dst != "eax") code << "    mov eax, " << dst << "\n";
                    code << "    idiv ecx\n";
                    if (dst != "eax") code << "    mov " << dst << ", eax\n";
                    code << "    pop rdx\n";
                } else {
                    code << "    push edx\n    xor edx, edx\n";
                    code << "    mov eax, " << dst << "\n";
                    code << "    idiv ecx\n";
                    code << "    mov " << dst << ", eax\n";
                    code << "    pop edx\n";
                }
            }
        } else {
            // Right side is a simple value
            auto rhs = [&]()->std::string {
                if (is_reg(right_str)) return reg(right_str);
                if (is_num(right_str) || is_hex(right_str)) return right_str;
                auto it = var_lbl.find(right_str);
                if (it == var_lbl.end()) throw std::runtime_error("undefined variable '" + right_str + "'");
                return "dword [" + addr(it->second) + "]";
            };
            
            if (op == '+') {
                code << "    add " << dst << ", " << rhs() << "\n";
            } else if (op == '-') {
                code << "    sub " << dst << ", " << rhs() << "\n";
            } else if (op == '*') {
                // imul doesn't support mem operand directly in 64-bit, load to ecx first
                std::string rhs_val = rhs();
                if (rhs_val.find('[') != std::string::npos) {
                    code << "    mov ecx, " << rhs_val << "\n";
                    code << "    imul " << dst << ", ecx\n";
                } else {
                    code << "    imul " << dst << ", " << rhs_val << "\n";
                }
            } else if (op == '/') {
                if (macos_terminal) {
                    code << "    push rdx\n";
                    code << "    xor edx, edx\n";
                    code << "    mov ecx, " << rhs() << "\n";
                    if (dst != "eax") code << "    mov eax, " << dst << "\n";
                    code << "    idiv ecx\n";
                    if (dst != "eax") code << "    mov " << dst << ", eax\n";
                    code << "    pop rdx\n";
                } else {
                    code << "    push edx\n    xor edx, edx\n    mov ecx, " << rhs() << "\n";
                    if (dst != "eax") code << "    mov eax, " << dst << "\n";
                    code << "    idiv ecx\n";
                    if (dst != "eax") code << "    mov " << dst << ", eax\n";
                    code << "    pop edx\n";
                }
            }
        }
    }

    void gen_var(VarDecl* v){
        std::string lb="var_"+v->name;
        var_lbl[v->name]=lb;
        var_type[v->name]=v->type;  // Store variable type
        var_is_ptr[v->name]=(v->type=="string"||v->type=="pointer"||v->type.find('*')==0);
        var_on_heap[v->name]=false;  // By default, variables are on stack/data section
        if(v->is_const) const_declared.insert(v->name);
        else declared.insert(v->name);
        if(v->is_arr){
            int esz=(v->type=="u8")?1:4;
            data<<"    "<<lb<<": times "<<v->arr_size*esz<<" db 0\n"; return;
        }
        if(v->type=="string"){
            std::string sl="str_"+std::to_string(scnt++);
            if(!v->init.empty()){
                std::string s=v->init;
                if(s.size()>=2 && s.front()=='"' && s.back()=='"') s=s.substr(1,s.size()-2);
                data<<"    "<<sl<<": db ";
                for(size_t i=0;i<s.size();++i){
                    data<<static_cast<int>(static_cast<unsigned char>(s[i]))<<", ";
                }
                data<<"0\n";
                if(macos_terminal){
                    data<<"    align 8\n";
                    data<<"    "<<lb<<": dq "<<sl<<"\n";
                }
                else data<<"    "<<lb<<": dd "<<sl<<"\n";
            } else {
                if(macos_terminal){
                    data<<"    align 8\n";
                    data<<"    "<<lb<<": dq 0\n";
                }
                else data<<"    "<<lb<<": dd 0\n";
            }
            return;
        }
        // Check if type is a pointer (*i32, *string, etc.)
        if(v->type.find('*')==0){
            // Pointer variable
            if(v->init.find('&')==0){
                // Initialize with address: var ptr: *i32 = &x
                // Need runtime initialization for 64-bit
                std::string refvar = v->init.substr(1);
                if(macos_terminal){
                    data<<"    align 8\n";
                    data<<"    "<<lb<<": dq 0\n";
                    // Will be initialized at runtime
                } else {
                    data<<"    "<<lb<<": dd var_"+refvar+"\n";
                }
            } else if(v->init=="0" || v->init.empty()){
                // Null or uninitialized pointer
                if(macos_terminal){
                    data<<"    align 8\n";
                    data<<"    "<<lb<<": dq "<<(v->init.empty()?"0":v->init)<<"\n";
                } else {
                    data<<"    "<<lb<<": dd "<<(v->init.empty()?"0":v->init)<<"\n";
                }
            } else {
                // Other initializer
                if(macos_terminal){
                    data<<"    align 8\n";
                    data<<"    "<<lb<<": dq "<<v->init<<"\n";
                } else {
                    data<<"    "<<lb<<": dd "<<v->init<<"\n";
                }
            }
            return;
        }
        // Check if type is a struct
        auto sit = struct_sizes.find(v->type);
        if(sit != struct_sizes.end()){
            // Variable of struct type - allocate space for all fields
            data<<"    "<<lb<<": times "<<sit->second<<" db 0\n";
            return;
        }
        // Check if initializer is dereference: *ptr - need runtime initialization
        if(v->init.find('*')==0){
            // Runtime initialization required - initialize to 0 and assign later
            std::string ptrname = v->init.substr(1);
            // Use proper size based on type
            if(macos_terminal && (v->type=="i64"||v->type=="string"||v->type=="pointer"||v->type.find('*')==0)){
                data<<"    align 8\n";
                data<<"    "<<lb<<": dq 0\n";
            } else {
                data<<"    "<<lb<<": dd 0\n";
            }
            // Store the initialization for later - will be handled in gen_section
            // For now, just initialize to 0
            return;
        }
        if(macos_terminal && (v->type=="pointer"||v->type.find('*')==0)){
            data<<"    align 8\n";
            data<<"    "<<lb<<": dq "<<(v->init.empty()?"0":v->init)<<"\n";
        } else {
            data<<"    "<<lb<<": dd "<<(v->init.empty()?"0":v->init)<<"\n";
        }
    }

    void gen_struct(StructDecl* s){
        // Register struct layout for field offset calculations
        int offset = 0;
        for(auto& f : s->fields){
            int fsize = 4;  // default size
            if(f.second == "u8") fsize = 1;
            else if(f.second == "i64" || f.second == "string" || f.second == "pointer") fsize = 8;
            else if(f.second == "i32") fsize = 4;
            struct_field_offsets[s->name][f.first] = offset;
            offset += fsize;
        }
        struct_sizes[s->name] = offset;
    }

    void gen_display(DisplayNode* d){
            auto it=var_lbl.find(d->var);
            if(it==var_lbl.end()){warn("display: unknown variable '"+d->var+"'");return;}
            
            // Check variable type - if i32/i64, use printnum instead
            auto tit = var_type.find(d->var);
            if(tit != var_type.end()){
                std::string type = tit->second;
                // If it's a numeric type (i32, i64, u8), use printnum
                if(type == "i32" || type == "i64" || type == "u8"){
                    PrintNumNode pn;
                    pn.var = d->var;
                    gen_printnum(&pn);
                    return;
                }
            }
            
            if(bare_metal){
            std::string L=lbl("disp");
            code<<"    mov esi, dword ["<<it->second<<"]\n";
            code<<"    mov edi, dword [__defacto_cursor]\n";
            code<<L<<"_loop:\n";
            code<<"    movzx eax, byte [esi]\n";
            code<<"    test al, al\n";
            code<<"    jz "<<L<<"_done\n";
            code<<"    cmp al, 10\n";
            code<<"    je "<<L<<"_nl\n";
            code<<"    mov byte [0xB8000 + edi], al\n";
            code<<"    mov bl, byte [__defacto_attr]\n";
            code<<"    mov byte [0xB8000 + edi + 1], bl\n";
            code<<"    add edi, 2\n";
            code<<"    add esi, 1\n";
            code<<"    jmp "<<L<<"_loop\n";
            code<<L<<"_nl:\n";
            code<<"    xor edx, edx\n";
            code<<"    mov eax, edi\n";
            code<<"    mov ecx, 160\n";
            code<<"    div ecx\n";
            code<<"    inc eax\n";
            code<<"    imul eax, eax, 160\n";
            code<<"    mov edi, eax\n";
            code<<"    add esi, 1\n";
            code<<"    jmp "<<L<<"_loop\n";
            code<<L<<"_done:\n";
            code<<"    mov dword [__defacto_cursor], edi\n";
        } else {
            std::string L=lbl("print");
            if(macos_terminal){
                code<<"    mov rsi, qword ["<<addr(it->second)<<"]\n";
                code<<"    mov rcx, rsi\n";
            } else {
                code<<"    mov esi, dword ["<<addr(it->second)<<"]\n";
                code<<"    mov ecx, esi\n";
            }
            code<<L<<"_len:\n";
            if(macos_terminal) code<<"    cmp byte [rcx], 0\n";
            else code<<"    cmp byte [ecx], 0\n";
            code<<"    je "<<L<<"_write\n";
            if(macos_terminal) code<<"    inc rcx\n";
            else code<<"    inc ecx\n";
            code<<"    jmp "<<L<<"_len\n";
            code<<L<<"_write:\n";
            if(macos_terminal) code<<"    sub rcx, rsi\n";
            else code<<"    sub ecx, esi\n";
            if(macos_terminal){
                code<<"    mov rax, 0x2000004\n";
                code<<"    mov rdi, 1\n";
                code<<"    mov rsi, qword ["<<addr(it->second)<<"]\n";
                code<<"    mov rdx, rcx\n";
                code<<"    syscall\n";
                code<<"    mov rax, 0x2000004\n";
                code<<"    mov rdi, 1\n";
                code<<"    lea rsi, ["<<addr(L+"_nl")<<"]\n";
                code<<"    mov rdx, 1\n";
                code<<"    syscall\n";
            } else {
                code<<"    mov eax, 4\n";
                code<<"    mov ebx, 1\n";
                code<<"    mov edx, ecx\n";
                code<<"    int 0x80\n";
                code<<"    mov eax, 4\n";
                code<<"    mov ebx, 1\n";
                code<<"    mov ecx, "<<L<<"_nl\n";
                code<<"    mov edx, 1\n";
                code<<"    int 0x80\n";
            }
            data<<"    "<<L<<"_nl: db 10\n";
        }
    }

    void gen_printnum(PrintNumNode* p){
        auto it = var_lbl.find(p->var);
        if(it == var_lbl.end()){
            warn("printnum: unknown variable '"+p->var+"'");
            return;
        }
        
        std::string L = lbl("pnum");
        
        if(bare_metal){
            // Bare-metal: вывод числа через VGA память
            // Алгоритм: делим на 10, получаем цифры, конвертируем в ASCII
            code<<"    mov eax, dword ["<<addr(it->second)<<"]\n";
            code<<"    mov ecx, 10\n";
            code<<"    mov edi, dword [__defacto_cursor]\n";
            code<<"    xor ebx, ebx  ; счетчик цифр\n";
            code<<L<<"_div:\n";
            code<<"    xor edx, edx\n";
            code<<"    div ecx\n";
            code<<"    push dx  ; сохраняем цифру\n";
            code<<"    inc ebx\n";
            code<<"    test eax, eax\n";
            code<<"    jnz "<<L<<"_div\n";
            code<<L<<"_print:\n";
            code<<"    pop dx\n";
            code<<"    add dl, 48  ; конвертируем в ASCII\n";
            code<<"    mov [0xB8000 + edi], dl\n";
            code<<"    mov bl, byte [__defacto_attr]\n";
            code<<"    mov [0xB8000 + edi + 1], bl\n";
            code<<"    add edi, 2\n";
            code<<"    dec ebx\n";
            code<<"    jnz "<<L<<"_print\n";
            code<<"    mov dword [__defacto_cursor], edi\n";
        } else if(macos_terminal){
            // macOS terminal (64-bit): конвертация числа в строку и вывод
            code<<"    mov eax, dword ["<<addr(it->second)<<"]\n";
            code<<"    mov ecx, 10\n";
            code<<"    sub rsp, 16  ; буфер\n";
            code<<"    mov rdi, rsp\n";
            code<<"    mov byte [rdi + 15], 0  ; null терминатор\n";
            code<<"    mov ebx, 14  ; позиция\n";
            code<<"    xor esi, esi  ; счетчик цифр\n";
            code<<L<<"_div:\n";
            code<<"    xor edx, edx\n";
            code<<"    div ecx\n";
            code<<"    add dl, 48\n";
            code<<"    mov [rdi + rbx], dl\n";
            code<<"    dec ebx\n";
            code<<"    inc esi\n";
            code<<"    test eax, eax\n";
            code<<"    jnz "<<L<<"_div\n";
            code<<"    inc ebx  ; корректировка\n";
            code<<"    mov rsi, rdi\n";
            code<<"    add rsi, rbx\n";
            code<<"    mov rcx, rsi\n";
            code<<L<<"_len:\n";
            code<<"    cmp byte [rcx], 0\n";
            code<<"    je "<<L<<"_write\n";
            code<<"    inc rcx\n";
            code<<"    jmp "<<L<<"_len\n";
            code<<L<<"_write:\n";
            code<<"    sub rcx, rsi\n";
            code<<"    mov rax, 0x2000004\n";
            code<<"    mov rdi, 1\n";
            code<<"    mov rdx, rcx\n";
            code<<"    syscall\n";
            code<<"    add rsp, 16\n";
            // newline
            code<<"    mov rax, 0x2000004\n";
            code<<"    mov rdi, 1\n";
            code<<"    lea rsi, ["<<addr(L+"_nl")<<"]\n";
            code<<"    mov rdx, 1\n";
            code<<"    syscall\n";
            data<<"    "<<L<<"_nl: db 10\n";
        } else {
            // Linux terminal: конвертация числа в строку и вывод
            code<<"    mov eax, dword ["<<addr(it->second)<<"]\n";
            code<<"    mov ecx, 10\n";
            code<<"    sub esp, 16\n";
            code<<"    mov edi, esp\n";
            code<<"    mov byte [edi + 15], 0\n";
            code<<"    mov ebx, 14\n";
            code<<"    xor esi, esi\n";
            code<<L<<"_div:\n";
            code<<"    xor edx, edx\n";
            code<<"    div ecx\n";
            code<<"    add dl, 48\n";
            code<<"    mov [edi + ebx], dl\n";
            code<<"    dec ebx\n";
            code<<"    inc esi\n";
            code<<"    test eax, eax\n";
            code<<"    jnz "<<L<<"_div\n";
            code<<"    inc ebx\n";
            code<<"    mov eax, 4\n";
            code<<"    mov ebx, 1\n";
            code<<"    mov ecx, edi\n";
            code<<"    add ecx, ebx\n";
            code<<"    mov edx, esi\n";
            code<<"    int 0x80\n";
            code<<"    add esp, 16\n";
            // newline
            code<<"    mov eax, 4\n";
            code<<"    mov ebx, 1\n";
            code<<"    mov ecx, "<<L<<"_nl\n";
            code<<"    mov edx, 1\n";
            code<<"    int 0x80\n";
            data<<"    "<<L<<"_nl: db 10\n";
        }
    }

    void gen_color(ColorNode* c){
        if(!bare_metal) return;
        const std::string& v=c->value;
        if(is_num(v)||is_hex(v)){
            code<<"    mov al, "<<v<<"\n";
            code<<"    mov byte [__defacto_attr], al\n";
            return;
        }
        if(is_reg(v)){
            code<<"    mov eax, "<<reg(v)<<"\n";
            code<<"    mov byte [__defacto_attr], al\n";
            return;
        }
        auto it=var_lbl.find(v);
        if(it==var_lbl.end()) throw std::runtime_error("color: undefined variable '"+v+"'");
        code<<"    mov eax, dword ["<<it->second<<"]\n";
        code<<"    mov byte [__defacto_attr], al\n";
    }

    void gen_readkey(ReadKeyNode* k){
        auto it=var_lbl.find(k->var);
        if(it==var_lbl.end()) throw std::runtime_error("readkey: undefined variable '"+k->var+"'");
        if(!bare_metal){
            code<<"    mov dword ["<<addr(it->second)<<"], 0\n";
            return;
        }
        std::string L=lbl("key");
        code<<L<<"_wait:\n";
        code<<"    in al, 0x64\n";
        code<<"    test al, 1\n";
        code<<"    jz "<<L<<"_wait\n";
        code<<"    in al, 0x60\n";
        code<<"    cmp al, 0x80\n";
        code<<"    jae "<<L<<"_wait\n";
        code<<"    cmp al, 0x02\n";
        code<<"    je "<<L<<"_1\n";
        code<<"    cmp al, 0x03\n";
        code<<"    je "<<L<<"_2\n";
        code<<"    cmp al, 0x04\n";
        code<<"    je "<<L<<"_3\n";
        code<<"    cmp al, 0x05\n";
        code<<"    je "<<L<<"_4\n";
        code<<"    cmp al, 0x06\n";
        code<<"    je "<<L<<"_5\n";
        code<<"    jmp "<<L<<"_wait\n";
        code<<L<<"_1:\n";
        code<<"    mov eax, 49\n";
        code<<"    jmp "<<L<<"_done\n";
        code<<L<<"_2:\n";
        code<<"    mov eax, 50\n";
        code<<"    jmp "<<L<<"_done\n";
        code<<L<<"_3:\n";
        code<<"    mov eax, 51\n";
        code<<"    jmp "<<L<<"_done\n";
        code<<L<<"_4:\n";
        code<<"    mov eax, 52\n";
        code<<"    jmp "<<L<<"_done\n";
        code<<L<<"_5:\n";
        code<<"    mov eax, 53\n";
        code<<L<<"_done:\n";
        code<<"    mov dword ["<<it->second<<"], eax\n";
    }

    void gen_readchar(ReadCharNode* k){
        auto it = var_lbl.find(k->var);
        if(it == var_lbl.end()) throw std::runtime_error("readchar: undefined variable '"+k->var+"'");
        
        if(!bare_metal){
            // Terminal mode: читаем 1 байт со stdin
            if(macos_terminal){
                // macOS: sys_read
                code<<"    mov rax, 0x2000003\n";
                code<<"    mov rdi, 0  ; stdin\n";
                code<<"    sub rsp, 8\n";
                code<<"    mov rsi, rsp  ; буфер на стеке\n";
                code<<"    mov rdx, 1  ; 1 байт\n";
                code<<"    syscall\n";
                code<<"    mov eax, dword [rsp]\n";
                code<<"    add rsp, 8\n";
                code<<"    and eax, 0xFF  ; только 1 байт\n";
                code<<"    mov dword ["<<addr(it->second)<<"], eax\n";
            } else {
                // Linux: sys_read
                code<<"    mov eax, 3\n";
                code<<"    mov ebx, 0  ; stdin\n";
                code<<"    sub esp, 4\n";
                code<<"    mov ecx, esp  ; буфер\n";
                code<<"    mov edx, 1  ; 1 байт\n";
                code<<"    int 0x80\n";
                code<<"    mov eax, dword [esp]\n";
                code<<"    add esp, 4\n";
                code<<"    and eax, 0xFF\n";
                code<<"    mov dword ["<<addr(it->second)<<"], eax\n";
            }
            return;
        }
        
        // Bare-metal: опрос клавиатуры
        std::string L=lbl("chr");
        code<<L<<"_wait:\n";
        code<<"    in al, 0x64\n";
        code<<"    test al, 1\n";
        code<<"    jz "<<L<<"_wait\n";
        code<<"    in al, 0x60\n";
        code<<"    cmp al, 0x80\n";
        code<<"    jae "<<L<<"_wait\n";
        code<<"    cmp al, 0x0E\n";
        code<<"    je "<<L<<"_bs\n";
        code<<"    cmp al, 0x1C\n";
        code<<"    je "<<L<<"_enter\n";
        code<<"    cmp al, 0x39\n";
        code<<"    je "<<L<<"_space\n";
        code<<"    cmp al, 0x02\n";
        code<<"    je "<<L<<"_1\n";
        code<<"    cmp al, 0x03\n";
        code<<"    je "<<L<<"_2\n";
        code<<"    cmp al, 0x04\n";
        code<<"    je "<<L<<"_3\n";
        code<<"    cmp al, 0x05\n";
        code<<"    je "<<L<<"_4\n";
        code<<"    cmp al, 0x06\n";
        code<<"    je "<<L<<"_5\n";
        code<<"    cmp al, 0x07\n";
        code<<"    je "<<L<<"_6\n";
        code<<"    cmp al, 0x08\n";
        code<<"    je "<<L<<"_7\n";
        code<<"    cmp al, 0x09\n";
        code<<"    je "<<L<<"_8\n";
        code<<"    cmp al, 0x0A\n";
        code<<"    je "<<L<<"_9\n";
        code<<"    cmp al, 0x0B\n";
        code<<"    je "<<L<<"_0\n";
        code<<"    cmp al, 0x10\n";
        code<<"    je "<<L<<"_q\n";
        code<<"    cmp al, 0x11\n";
        code<<"    je "<<L<<"_w\n";
        code<<"    cmp al, 0x12\n";
        code<<"    je "<<L<<"_e\n";
        code<<"    cmp al, 0x13\n";
        code<<"    je "<<L<<"_r\n";
        code<<"    cmp al, 0x14\n";
        code<<"    je "<<L<<"_t\n";
        code<<"    cmp al, 0x15\n";
        code<<"    je "<<L<<"_y\n";
        code<<"    cmp al, 0x16\n";
        code<<"    je "<<L<<"_u\n";
        code<<"    cmp al, 0x17\n";
        code<<"    je "<<L<<"_i\n";
        code<<"    cmp al, 0x18\n";
        code<<"    je "<<L<<"_o\n";
        code<<"    cmp al, 0x19\n";
        code<<"    je "<<L<<"_p\n";
        code<<"    cmp al, 0x1E\n";
        code<<"    je "<<L<<"_a\n";
        code<<"    cmp al, 0x1F\n";
        code<<"    je "<<L<<"_s\n";
        code<<"    cmp al, 0x20\n";
        code<<"    je "<<L<<"_d\n";
        code<<"    cmp al, 0x21\n";
        code<<"    je "<<L<<"_f\n";
        code<<"    cmp al, 0x22\n";
        code<<"    je "<<L<<"_g\n";
        code<<"    cmp al, 0x23\n";
        code<<"    je "<<L<<"_h\n";
        code<<"    cmp al, 0x24\n";
        code<<"    je "<<L<<"_j\n";
        code<<"    cmp al, 0x25\n";
        code<<"    je "<<L<<"_k\n";
        code<<"    cmp al, 0x26\n";
        code<<"    je "<<L<<"_l\n";
        code<<"    cmp al, 0x2C\n";
        code<<"    je "<<L<<"_z\n";
        code<<"    cmp al, 0x2D\n";
        code<<"    je "<<L<<"_x\n";
        code<<"    cmp al, 0x2E\n";
        code<<"    je "<<L<<"_c\n";
        code<<"    cmp al, 0x2F\n";
        code<<"    je "<<L<<"_v\n";
        code<<"    cmp al, 0x30\n";
        code<<"    je "<<L<<"_b\n";
        code<<"    cmp al, 0x31\n";
        code<<"    je "<<L<<"_n\n";
        code<<"    cmp al, 0x32\n";
        code<<"    je "<<L<<"_m\n";
        code<<"    jmp "<<L<<"_wait\n";
        code<<L<<"_bs:\n    mov eax, 8\n    jmp "<<L<<"_done\n";
        code<<L<<"_enter:\n    mov eax, 10\n    jmp "<<L<<"_done\n";
        code<<L<<"_space:\n    mov eax, 32\n    jmp "<<L<<"_done\n";
        code<<L<<"_1:\n    mov eax, 49\n    jmp "<<L<<"_done\n";
        code<<L<<"_2:\n    mov eax, 50\n    jmp "<<L<<"_done\n";
        code<<L<<"_3:\n    mov eax, 51\n    jmp "<<L<<"_done\n";
        code<<L<<"_4:\n    mov eax, 52\n    jmp "<<L<<"_done\n";
        code<<L<<"_5:\n    mov eax, 53\n    jmp "<<L<<"_done\n";
        code<<L<<"_6:\n    mov eax, 54\n    jmp "<<L<<"_done\n";
        code<<L<<"_7:\n    mov eax, 55\n    jmp "<<L<<"_done\n";
        code<<L<<"_8:\n    mov eax, 56\n    jmp "<<L<<"_done\n";
        code<<L<<"_9:\n    mov eax, 57\n    jmp "<<L<<"_done\n";
        code<<L<<"_0:\n    mov eax, 48\n    jmp "<<L<<"_done\n";
        code<<L<<"_q:\n    mov eax, 113\n    jmp "<<L<<"_done\n";
        code<<L<<"_w:\n    mov eax, 119\n    jmp "<<L<<"_done\n";
        code<<L<<"_e:\n    mov eax, 101\n    jmp "<<L<<"_done\n";
        code<<L<<"_r:\n    mov eax, 114\n    jmp "<<L<<"_done\n";
        code<<L<<"_t:\n    mov eax, 116\n    jmp "<<L<<"_done\n";
        code<<L<<"_y:\n    mov eax, 121\n    jmp "<<L<<"_done\n";
        code<<L<<"_u:\n    mov eax, 117\n    jmp "<<L<<"_done\n";
        code<<L<<"_i:\n    mov eax, 105\n    jmp "<<L<<"_done\n";
        code<<L<<"_o:\n    mov eax, 111\n    jmp "<<L<<"_done\n";
        code<<L<<"_p:\n    mov eax, 112\n    jmp "<<L<<"_done\n";
        code<<L<<"_a:\n    mov eax, 97\n    jmp "<<L<<"_done\n";
        code<<L<<"_s:\n    mov eax, 115\n    jmp "<<L<<"_done\n";
        code<<L<<"_d:\n    mov eax, 100\n    jmp "<<L<<"_done\n";
        code<<L<<"_f:\n    mov eax, 102\n    jmp "<<L<<"_done\n";
        code<<L<<"_g:\n    mov eax, 103\n    jmp "<<L<<"_done\n";
        code<<L<<"_h:\n    mov eax, 104\n    jmp "<<L<<"_done\n";
        code<<L<<"_j:\n    mov eax, 106\n    jmp "<<L<<"_done\n";
        code<<L<<"_k:\n    mov eax, 107\n    jmp "<<L<<"_done\n";
        code<<L<<"_l:\n    mov eax, 108\n    jmp "<<L<<"_done\n";
        code<<L<<"_z:\n    mov eax, 122\n    jmp "<<L<<"_done\n";
        code<<L<<"_x:\n    mov eax, 120\n    jmp "<<L<<"_done\n";
        code<<L<<"_c:\n    mov eax, 99\n    jmp "<<L<<"_done\n";
        code<<L<<"_v:\n    mov eax, 118\n    jmp "<<L<<"_done\n";
        code<<L<<"_b:\n    mov eax, 98\n    jmp "<<L<<"_done\n";
        code<<L<<"_n:\n    mov eax, 110\n    jmp "<<L<<"_done\n";
        code<<L<<"_m:\n    mov eax, 109\n";
        code<<L<<"_done:\n";
        code<<"    mov dword ["<<it->second<<"], eax\n";
    }

    void gen_putchar(PutCharNode* p){
        if(!bare_metal) return;
        const std::string& v=p->value;
        if(is_reg(v)) code<<"    mov eax, "<<reg(v)<<"\n";
        else if(is_num(v)||is_hex(v)) code<<"    mov eax, "<<v<<"\n";
        else{
            auto it=var_lbl.find(v);
            if(it==var_lbl.end()) throw std::runtime_error("putchar: undefined variable '"+v+"'");
            code<<"    mov eax, dword ["<<it->second<<"]\n";
        }
        std::string L=lbl("putc");
        code<<"    mov edi, dword [__defacto_cursor]\n";
        code<<"    cmp al, 10\n";
        code<<"    je "<<L<<"_nl\n";
        code<<"    cmp al, 8\n";
        code<<"    je "<<L<<"_bs\n";
        code<<"    mov byte [0xB8000 + edi], al\n";
        code<<"    mov bl, byte [__defacto_attr]\n";
        code<<"    mov byte [0xB8000 + edi + 1], bl\n";
        code<<"    add edi, 2\n";
        code<<"    mov dword [__defacto_cursor], edi\n";
        code<<"    jmp "<<L<<"_done\n";
        code<<L<<"_nl:\n";
        code<<"    xor edx, edx\n";
        code<<"    mov eax, edi\n";
        code<<"    mov ecx, 160\n";
        code<<"    div ecx\n";
        code<<"    inc eax\n";
        code<<"    imul eax, eax, 160\n";
        code<<"    mov dword [__defacto_cursor], eax\n";
        code<<"    jmp "<<L<<"_done\n";
        code<<L<<"_bs:\n";
        code<<"    cmp edi, 2\n";
        code<<"    jb "<<L<<"_done\n";
        code<<"    sub edi, 2\n";
        code<<"    mov byte [0xB8000 + edi], 32\n";
        code<<"    mov bl, byte [__defacto_attr]\n";
        code<<"    mov byte [0xB8000 + edi + 1], bl\n";
        code<<"    mov dword [__defacto_cursor], edi\n";
        code<<L<<"_done:\n";
    }

    void gen_clear(ClearNode*){
        if(!bare_metal) return;
        code<<"    mov edi, 0xB8000\n";
        code<<"    mov al, 32\n";
        code<<"    mov ah, byte [__defacto_attr]\n";
        code<<"    mov ecx, 2000\n";
        code<<"    rep stosw\n";
        code<<"    mov dword [__defacto_cursor], 0\n";
    }

    void gen_reboot(RebootNode*){
        if(!bare_metal) return;
        std::string L=lbl("reboot");
        code<<L<<"_wait:\n";
        code<<"    in al, 0x64\n";
        code<<"    test al, 2\n";
        code<<"    jnz "<<L<<"_wait\n";
        code<<"    mov al, 0xFE\n";
        code<<"    out 0x64, al\n";
        code<<L<<"_hang:\n";
        code<<"    hlt\n";
        code<<"    jmp "<<L<<"_hang\n";
    }

    void gen_assign(Assign* a){
        if(const_declared.count(a->target))
            throw std::runtime_error("cannot assign to const '"+a->target+"'");
        if(a->is_reg){
            std::string dst=reg(a->target);
            if(a->value[0]=='(') expr(dst,a->value); else load(dst,a->value); return;
        }
        // Check for dereference assignment: *ptr = value
        if(a->target.size() > 0 && a->target[0] == '*') {
            std::string ptrname = a->target.substr(1);
            load("eax", a->value);
            store("eax", "*" + ptrname);
            return;
        }
        // Check for struct field access: struct.field
        auto dot_pos = a->target.find('.');
        if(dot_pos != std::string::npos){
            std::string struct_var = a->target.substr(0, dot_pos);
            std::string field_name = a->target.substr(dot_pos + 1);
            // Look up the variable's type
            auto tvit = var_type.find(struct_var);
            if(tvit == var_type.end()){
                throw std::runtime_error("undefined variable '"+struct_var+"'");
            }
            std::string struct_type = tvit->second;
            // Look up struct layout by type
            auto sit = struct_field_offsets.find(struct_type);
            if(sit == struct_field_offsets.end()){
                // Not a struct, treat as regular variable (backward compatibility)
            } else {
                auto fit = sit->second.find(field_name);
                if(fit == sit->second.end()){
                    throw std::runtime_error("unknown field '"+field_name+"' in struct '"+struct_type+"'");
                }
                auto vit = var_lbl.find(struct_var);
                if(vit == var_lbl.end()){
                    throw std::runtime_error("undefined struct variable '"+struct_var+"'");
                }
                int offset = fit->second;
                load("eax", a->value);
                if(offset == 0){
                    if(macos_terminal){
                        code<<"    mov qword [rel "<<vit->second<<"], rax\n";
                    } else {
                        code<<"    mov dword ["<<addr(vit->second)<<"], eax\n";
                    }
                } else {
                    if(macos_terminal){
                        // 64-bit: load base address into rbx, then store
                        code<<"    lea rbx, [rel "<<vit->second<<"]\n";
                        code<<"    mov dword [rbx + "<<offset<<"], eax\n";
                    } else {
                        code<<"    mov ebx, "<<addr(vit->second)<<"\n";
                        code<<"    mov dword [ebx + "<<offset<<"], eax\n";
                    }
                }
                return;
            }
        }
        if(a->is_arr){
            auto it=var_lbl.find(a->target);
            if(it==var_lbl.end()) throw std::runtime_error("undefined array '"+a->target+"'");
            load("eax",a->value);
            if(is_reg(a->idx)) code<<"    mov ecx, "<<reg(a->idx)<<"\n";
            else if(is_num(a->idx)) code<<"    mov ecx, "<<a->idx<<"\n";
            else{auto jt=var_lbl.find(a->idx);code<<"    mov ecx, dword ["<<addr(jt->second)<<"]\n";}
            code<<"    mov dword ["<<addr(it->second)<<" + ecx*4], eax\n"; return;
        }
        if(a->value[0]=='('){expr("eax",a->value);store("eax",a->target);}
        else if(is_reg(a->value)) store(reg(a->value),a->target);
        else if(is_num(a->value)||is_hex(a->value)){
            auto it=var_lbl.find(a->target);
            if(it==var_lbl.end()) throw std::runtime_error("undefined variable '"+a->target+"'");
            code<<"    mov dword ["<<addr(it->second)<<"], "<<a->value<<"\n";
        } else{load("eax",a->value);store("eax",a->target);}
    }

    void gen_loop(LoopNode* l){
        std::string ls=lbl("loop_s"),le=lbl("loop_e");
        loop_ends.push_back(le);
        code<<ls<<":\n";
        for(auto& s:l->body) gen_stmt(s.get());
        code<<"    jmp "<<ls<<"\n"<<le<<":\n";
        loop_ends.pop_back();
    }

    void gen_while(WhileNode* w){
        std::string ws=lbl("while_s"), we=lbl("while_e");
        code<<ws<<":\n";
        // Check condition
        load("eax", w->left);
        if(is_num(w->right)) code<<"    cmp eax, "<<w->right<<"\n";
        else if(is_reg(w->right)) code<<"    cmp eax, "<<reg(w->right)<<"\n";
        else {
            auto it=var_lbl.find(w->right);
            code<<"    cmp eax, dword ["<<addr(it->second)<<"]\n";
        }
        // Jump based on operator
        if(w->op=="==") code<<"    je "<<we<<"\n";
        else if(w->op=="!=") code<<"    jne "<<we<<"\n";
        else if(w->op=="<") code<<"    jge "<<we<<"\n";
        else if(w->op==">") code<<"    jle "<<we<<"\n";
        else if(w->op=="<=") code<<"    jg "<<we<<"\n";
        else if(w->op==">=") code<<"    jl "<<we<<"\n";
        // Body
        loop_ends.push_back(we);
        for(auto& s:w->body) gen_stmt(s.get());
        loop_ends.pop_back();
        code<<"    jmp "<<ws<<"\n"<<we<<":\n";
    }

    void gen_for(ForNode* f){
        std::string fs=lbl("for_s"), fe=lbl("for_e");
        // Init: var = value
        auto it=var_lbl.find(f->init_var);
        if(it==var_lbl.end()) throw std::runtime_error("undefined variable '"+f->init_var+"'");
        if(is_num(f->init_value)) code<<"    mov dword ["<<addr(it->second)<<"], "<<f->init_value<<"\n";
        else {
            load("eax", f->init_value);
            code<<"    mov dword ["<<addr(it->second)<<"], eax\n";
        }
        code<<fs<<":\n";
        // Check condition
        load("eax", f->cond_left);
        if(is_num(f->cond_right)) code<<"    cmp eax, "<<f->cond_right<<"\n";
        else if(is_reg(f->cond_right)) code<<"    cmp eax, "<<reg(f->cond_right)<<"\n";
        else {
            auto jt=var_lbl.find(f->cond_right);
            code<<"    cmp eax, dword ["<<addr(jt->second)<<"]\n";
        }
        // Jump based on operator
        if(f->cond_op=="==") code<<"    je "<<fe<<"\n";
        else if(f->cond_op=="!=") code<<"    jne "<<fe<<"\n";
        else if(f->cond_op=="<") code<<"    jge "<<fe<<"\n";
        else if(f->cond_op==">") code<<"    jle "<<fe<<"\n";
        else if(f->cond_op=="<=") code<<"    jg "<<fe<<"\n";
        else if(f->cond_op==">=") code<<"    jl "<<fe<<"\n";
        // Body
        loop_ends.push_back(fe);
        for(auto& s:f->body) gen_stmt(s.get());
        loop_ends.pop_back();
        // Step: var = step_value
        auto step_it=var_lbl.find(f->step_var);
        if(step_it==var_lbl.end()) throw std::runtime_error("undefined variable '"+f->step_var+"'");
        // Parse step expression and store
        load("eax", f->step_value);
        code<<"    mov dword ["<<addr(step_it->second)<<"], eax\n";
        code<<"    jmp "<<fs<<"\n"<<fe<<":\n";
    }

    void gen_if(IfNode* n){
        std::string L=lbl("if_skip"), Le=lbl("if_end");
        load("eax",n->left);
        if(is_num(n->right))      code<<"    cmp eax, "<<n->right<<"\n";
        else if(is_reg(n->right)) code<<"    cmp eax, "<<reg(n->right)<<"\n";
        else{auto it=var_lbl.find(n->right);code<<"    cmp eax, dword ["<<addr(it->second)<<"]\n";}
        if(n->op=="==")      code<<"    jne "<<L<<"\n";
        else if(n->op=="!=") code<<"    je  "<<L<<"\n";
        else if(n->op=="<")  code<<"    jge "<<L<<"\n";
        else if(n->op==">")  code<<"    jle "<<L<<"\n";
        else if(n->op=="<=") code<<"    jg  "<<L<<"\n";
        else if(n->op==">=") code<<"    jl  "<<L<<"\n";
        for(auto& s:n->then_body) gen_stmt(s.get());
        if(!n->else_body.empty()){
            code<<"    jmp "<<Le<<"\n";
            code<<L<<":\n";
            for(auto& s:n->else_body) gen_stmt(s.get());
            code<<Le<<":\n";
        } else {
            code<<L<<":\n";
        }
    }

    void gen_stmt(Node* n){
        if(!n) return;
        switch(n->kind){
            case NT::ASSIGN:   gen_assign(static_cast<Assign*>(n)); break;
            case NT::REG_OP:   {auto r=static_cast<RegOp*>(n); if(r->op=="MOV") load(reg(r->target),r->source); break;}
            case NT::LOOP:     gen_loop(static_cast<LoopNode*>(n)); break;
            case NT::WHILE:    gen_while(static_cast<WhileNode*>(n)); break;
            case NT::FOR:      gen_for(static_cast<ForNode*>(n)); break;
            case NT::IF_STMT:  gen_if(static_cast<IfNode*>(n)); break;
            case NT::DISPLAY:  gen_display(static_cast<DisplayNode*>(n)); break;
            case NT::PRINTNUM: gen_printnum(static_cast<PrintNumNode*>(n)); break;
            case NT::COLOR:    gen_color(static_cast<ColorNode*>(n)); break;
            case NT::READKEY:  gen_readkey(static_cast<ReadKeyNode*>(n)); break;
            case NT::READCHAR: gen_readchar(static_cast<ReadCharNode*>(n)); break;
            case NT::PUTCHAR:  gen_putchar(static_cast<PutCharNode*>(n)); break;
            case NT::CLEAR:    gen_clear(static_cast<ClearNode*>(n)); break;
            case NT::REBOOT:   gen_reboot(static_cast<RebootNode*>(n)); break;
            case NT::FREE:     if(const_declared.count(static_cast<FreeNode*>(n)->var))
                                   throw std::runtime_error("cannot free const '"+static_cast<FreeNode*>(n)->var+"'");
                               freed.insert(static_cast<FreeNode*>(n)->var);
                               break;
            case NT::DEALLOC_NODE: {
                // Generate free() call for heap-allocated memory
                auto dn = static_cast<DeallocNode*>(n);
                auto it = var_lbl.find(dn->ptr);
                if(it != var_lbl.end()){
                    code<<"    push dword ["<<addr(it->second)<<"]\n";
                    code<<"    call free\n";
                    code<<"    add esp, 4\n";
                    code<<"    mov dword ["<<addr(it->second)<<"], 0\n";
                }
                break;
            }
            case NT::ALLOC_NODE: {
                // Generate malloc() call - result in EAX
                auto an = static_cast<AllocNode*>(n);
                std::string size = an->size;
                if(is_num(size)){
                    code<<"    push "<<size<<"\n";
                } else {
                    load("eax", size);
                    code<<"    push eax\n";
                }
                code<<"    call malloc\n";
                code<<"    add esp, 4\n";
                // Result (pointer) is in EAX
                break;
            }
            case NT::FUNC_CALL:{
                std::string nm=static_cast<FuncCall*>(n)->name;
                if(!nm.empty()&&nm[0]=='#') {
                    nm=nm.substr(1);
                    // Check if this is a driver call (keyboard, mouse, volume)
                    if(nm == "keyboard_driver" || nm == "mouse_driver" || nm == "volume_driver") {
                        // Extract driver type from name (e.g., "keyboard_driver" -> "keyboard")
                        std::string drv_type = nm.substr(0, nm.find("_driver"));
                        code<<"    call __defacto_drv_"<<drv_type<<"\n";
                    } else {
                        code<<"    call "<<nm<<"\n";
                    }
                } else {
                    code<<"    call "<<nm<<"\n";
                }
                break;
            }
            case NT::DRV_CALL: {
                auto dc=static_cast<DriverCall*>(n);
                if (dc->use_builtin) {
                    // Call built-in driver routine
                    std::string drv_name = dc->builtin_name;
                    // Remove # prefix if present
                    if (!drv_name.empty() && drv_name[0] == '#') {
                        drv_name = drv_name.substr(1);
                    }
                    code<<"    call __defacto_drv_"<<drv_name<<"\n";
                } else {
                    // Custom driver implementation
                    code<<"    call "<<dc->builtin_name<<"\n";
                }
                if (!dc->driver_target.empty()) {
                    code<<"    mov dword ["<<dc->driver_target<<"], eax\n";
                }
                break;
            }
            case NT::BREAK:    if(loop_ends.empty()) throw std::runtime_error("'stop' outside loop");
                               code<<"    jmp "<<loop_ends.back()<<"\n"; break;
            case NT::RETURN:   {
                auto rn = static_cast<ReturnNode*>(n);
                // Load return value into eax (if provided)
                if (!rn->value.empty()) {
                    load("eax", rn->value);
                }
                // For now, just return from function
                // TODO: implement proper function epilogue jump
                code<<"    mov esp, ebp\n    pop ebp\n    ret\n";
                break;
            }
            default: break;
        }
    }

    void gen_section(SectionNode* s){
        for(auto& d:s->decls) gen_var(static_cast<VarDecl*>(d.get()));

        // Handle address-of initializers FIRST: var ptr: *i32 = &x (runtime init for 64-bit)
        // This must come before dereference initializers
        for(auto& d:s->decls) {
            auto v = static_cast<VarDecl*>(d.get());
            if(v->init.find('&')==0 && v->type.find('*')==0){
                std::string refvar = v->init.substr(1);
                if(macos_terminal){
                    // 64-bit: load address into register and store
                    code<<"    lea rax, [rel var_"<<refvar<<"]\n";
                    code<<"    mov qword [rel var_"<<v->name<<"], rax\n";
                }
            }
        }

        // Handle dereference initializers SECOND: var y: i32 = *ptr
        for(auto& d:s->decls) {
            auto v = static_cast<VarDecl*>(d.get());
            if(v->init.find('*')==0){
                std::string ptrname = v->init.substr(1);
                // Generate: y = *ptr
                auto assign = std::make_unique<Assign>();
                assign->target = v->name;
                assign->value = "*" + ptrname;
                gen_assign(assign.get());
            }
        }

        for(auto& st:s->stmts) gen_stmt(st.get());
    }

    void gen_driver_section(DriverSectionNode* s){
        // Register driver constant so it doesn't need to be freed
        if (!s->driver_name.empty()) {
            driver_constants.insert(s->driver_name);
        }

        // Generate driver initialization code
        std::string driver_type = s->driver_type;
        // Remove # prefix if present
        if (!driver_type.empty() && driver_type[0] == '#') {
            driver_type = driver_type.substr(1);
        }

        // Generate built-in driver routines based on type
        // In terminal mode, drivers are stubs (no hardware access)
        if (driver_type == "keyboard") {
            code<<"\n__defacto_drv_keyboard:\n";
            code<<"    ; Built-in keyboard driver\n";
            if(!bare_metal){
                // Terminal mode: stub - no hardware access
                code<<"    ret\n";
            } else {
                code<<"    pushad\n";
                code<<"    call _init_keyboard\n";
                code<<"    popad\n";
                code<<"    ret\n";
            }
        } else if (driver_type == "mouse") {
            code<<"\n__defacto_drv_mouse:\n";
            code<<"    ; Built-in mouse driver\n";
            if(!bare_metal){
                // Terminal mode: stub - no hardware access
                code<<"    ret\n";
            } else {
                code<<"    pushad\n";
                code<<"    call _init_mouse\n";
                code<<"    popad\n";
                code<<"    ret\n";
            }
        } else if (driver_type == "volume") {
            code<<"\n__defacto_drv_volume:\n";
            code<<"    ; Built-in volume driver (PC speaker)\n";
            if(!bare_metal){
                // Terminal mode: stub - no hardware access
                code<<"    ret\n";
            } else {
                code<<"    pushad\n";
                code<<"    call _init_speaker\n";
                code<<"    popad\n";
                code<<"    ret\n";
            }
        }
    }

    void gen_func(FuncDecl* f){
        std::string nm=f->name;
        if(!nm.empty()&&nm[0]=='#') nm=nm.substr(1);
        std::string func_ret = lbl("func_ret");
        code<<"\n"<<nm<<":\n    push ebp\n    mov ebp, esp\n";
        gen_section(f->body.get());
        code<<func_ret<<":\n";
        code<<"    mov esp, ebp\n    pop ebp\n    ret\n";
    }

    void gen_auto_free(){
        // Automatically free all declared variables at the end of the section
        for(auto& v:declared){
            if(!freed.count(v) && !driver_constants.count(v)){
                auto it=var_lbl.find(v);
                if(it!=var_lbl.end()){
                    // Generate free code based on variable type
                    if(var_is_ptr[v]){
                        // For string/pointer types, free the allocated string data
                        code<<"    ; auto-free: "+v+"\n";
                        // String data is static, no runtime free needed for bare-metal
                    } else {
                        // For i32/i64/u8 types, just mark as freed (static allocation)
                        code<<"    ; auto-free: "+v+"\n";
                    }
                    freed.insert(v);
                }
            }
        }
    }

    void check_mem(){
        // Auto-free all variables - compiler handles memory management
        gen_auto_free();
    }

public:
    void set_mode(bool bm, bool macos=false){ 
        bare_metal=bm; 
        macos_terminal=macos;
        use_allocator = !bm;  // Use allocator in terminal mode
    }

    void emit(ProgramNode* prog, const std::string& out_path){
        code<<"global _start\n";
        
        // Add extern declarations for malloc/free in terminal mode
        if(!bare_metal && !macos_terminal){
            code<<"extern malloc\n";
            code<<"extern free\n";
            code<<"extern exit\n";
        }
        
        if(macos_terminal) code<<"section .text\n";
        code<<"_start:\n";
        
        // Setup stack frame for terminal mode
        if(!bare_metal && !macos_terminal){
            code<<"    push ebp\n";
            code<<"    mov ebp, esp\n";
        }

        // Generate struct definitions first
        for(auto& s:prog->structs) gen_struct(s.get());

        // Generate driver code first if present
        for(auto& s:prog->main_sec) {
            if (s->kind == NT::DRIVER_SECTION) {
                gen_driver_section(static_cast<DriverSectionNode*>(s.get()));
            }
        }

        for(auto& s:prog->main_sec) {
            if (s->kind == NT::SECTION) {
                gen_section(static_cast<SectionNode*>(s.get()));
            }
        }
        check_mem();

        if(bare_metal){
            code<<"\n.hang:\n    cli\n    hlt\n    jmp .hang\n";

            // Add built-in driver stubs
            code<<"\n; Built-in driver stubs\n";
            code<<"_init_keyboard:\n    ret\n";
            code<<"_init_mouse:\n    ret\n";
            code<<"_init_speaker:\n    ret\n";
        } else {
            if(macos_terminal){
                code<<"\n    mov rax, 0x2000001\n    xor rdi, rdi\n    syscall\n";
            } else {
                code<<"\n    mov eax, 1\n";
                code<<"    xor ebx, ebx\n";
                code<<"    int 0x80\n";
            }
        }

        for(auto& f:prog->functions) gen_func(static_cast<FuncDecl*>(f.get()));

        std::ofstream f(out_path);
        if(!f) throw std::runtime_error("cannot write '"+out_path+"'");

        if(bare_metal){
            f<<"[BITS 32]\n[ORG 0x1000]\n\n";
        } else {
            if(macos_terminal) f<<"[BITS 64]\nDEFAULT REL\n";
            else f<<"[BITS 32]\n";
        }
        f<<code.str()<<"\n";
        
        // Add data section
        if(!bare_metal) f<<"section .data\n";
        if(bare_metal){
            f<<"__defacto_cursor: dd 0\n";
            f<<"__defacto_attr: db 15\n";
        }
        f<<data.str()<<"\n";
        f.close();
        std::cout<<"asm: "<<out_path<<"\n";
    }
};
