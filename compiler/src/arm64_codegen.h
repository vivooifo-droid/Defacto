#pragma once
#include "defacto.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <map>

// ARM64 Code Generator for Defacto
// Supports macOS ARM64 and Linux ARM64

class ARM64CodeGen {
    std::ostringstream code;
    std::ostringstream data;
    std::ostringstream externs;

    std::map<std::string,std::string> var_lbl;
    std::map<std::string,bool> var_is_ptr;
    std::map<std::string,std::string> var_type;
    std::map<std::string, std::map<std::string, int>> struct_field_offsets;
    std::map<std::string, int> struct_sizes;
    std::set<std::string> declared, freed, const_declared;
    std::vector<std::string> loop_ends;
    int lcnt = 0, scnt = 0;
    bool macos_arm64 = true;  // true = macOS, false = Linux ARM64

    std::string lbl(const std::string& pfx="L") { return pfx+std::to_string(lcnt++); }

    // ARM64 register mapping
    // x0-x7: argument/return registers
    // x9-x15: temporary registers
    // x19-x28: callee-saved registers
    // x29: frame pointer (FP)
    // x30: link register (LR)
    // sp: stack pointer
    std::string reg(int n, bool wide=false) {
        if (n < 0 || n > 30) return "x0";
        if (wide) return "w" + std::to_string(n);  // 32-bit
        return "x" + std::to_string(n);  // 64-bit
    }

    bool is_reg(const std::string& s) {
        if (s.size() < 2) return false;
        if (s[0] == '#') {
            if (s[1] == 'R' && s.size() >= 3 && isdigit(s[2])) return true;
        }
        return false;
    }

    int reg_num(const std::string& s) {
        if (s.size() >= 3 && s[0] == '#' && s[1] == 'R') {
            return std::stoi(s.substr(2));
        }
        return 0;
    }

    bool is_num(const std::string& s) {
        return !s.empty() && (isdigit(s[0]) || (s.size() > 1 && s[0] == '-' && isdigit(s[1])));
    }

    bool is_hex(const std::string& s) {
        return s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
    }

    void load(const std::string& dst_reg, const std::string& src) {
        int dst = std::stoi(dst_reg.substr(1));  // Extract register number from xN
        
        if (is_num(src)) {
            int val = std::stoi(src);
            if (val >= 0 && val <= 0xFFFF) {
                code << "    mov " << reg(dst) << ", #" << val << "\n";
            } else {
                code << "    movz " << reg(dst) << ", #" << (val & 0xFFFF) << "\n";
                if (val > 0xFFFF) {
                    code << "    movk " << reg(dst) << ", #" << ((val >> 16) & 0xFFFF) << ", lsl #16\n";
                }
            }
        } else if (is_reg(src)) {
            int src_n = reg_num(src);
            code << "    mov " << reg(dst) << ", " << reg(src_n) << "\n";
        } else {
            // Load from memory
            auto it = var_lbl.find(src);
            if (it != var_lbl.end()) {
                code << "    adrp " << reg(dst) << ", " << it->second << "@PAGE\n";
                code << "    ldr " << reg(dst) << ", [" << reg(dst) << ", " << it->second << "@PAGEOFF]\n";
            }
        }
    }

    void store(const std::string& src_reg, const std::string& dst) {
        int src = std::stoi(src_reg.substr(1));
        auto it = var_lbl.find(dst);
        if (it != var_lbl.end()) {
            code << "    adrp x16, " << it->second << "@PAGE\n";
            code << "    str " << reg(src) << ", [x16, " << it->second << "@PAGEOFF]\n";
        }
    }

public:
    void set_mode(bool macos) {
        macos_arm64 = macos;
    }

    void emit(ProgramNode* prog, const std::string& out_path) {
        code << ".section __TEXT,__text\n";
        code << ".global _start\n";
        
        // Entry point
        code << "_start:\n";
        code << "    stp x29, x30, [sp, #-16]!\n";  // Save FP and LR
        code << "    mov x29, sp\n";
        
        // Generate struct definitions
        for(auto& s : prog->structs) gen_struct(s.get());
        
        // Generate main section
        for(auto& s : prog->main_sec) {
            if (s->kind == NT::SECTION) {
                gen_section(static_cast<SectionNode*>(s.get()));
            }
        }
        
        // Exit
        code << "    mov x0, #0\n";
        if (macos_arm64) {
            code << "    mov x16, #1\n";
            code << "    svc #0x80\n";
        } else {
            code << "    mov x8, #93\n";  // exit syscall
            code << "    svc #0\n";
        }
        
        // Generate functions
        for(auto& f : prog->functions) gen_func(static_cast<FuncDecl*>(f.get()));
        
        // Data section
        code << "\n.section __DATA,__data\n";
        code << data.str();
        
        // Write output
        std::ofstream f(out_path);
        if(!f) throw std::runtime_error("cannot write '"+out_path+"'");
        f << code.str() << "\n";
    }

    void gen_struct(StructDecl* s) {
        int offset = 0;
        for(auto& f : s->fields) {
            int fsize = 4;
            if(f.second == "u8") fsize = 1;
            else if(f.second == "i64" || f.second == "string" || f.second == "pointer") fsize = 8;
            else if(f.second == "i32") fsize = 4;
            struct_field_offsets[s->name][f.first] = offset;
            offset += fsize;
        }
        struct_sizes[s->name] = offset;
    }

    void gen_section(SectionNode* s) {
        // Generate declarations
        for(auto& d : s->decls) gen_var(static_cast<VarDecl*>(d.get()));
        
        // Generate statements
        for(auto& st : s->stmts) gen_stmt(st.get());
    }

    void gen_var(VarDecl* v) {
        std::string lb = "var_" + v->name;
        var_lbl[v->name] = lb;
        var_type[v->name] = v->type;
        var_is_ptr[v->name] = (v->type == "string" || v->type == "pointer" || v->type.find('*') == 0);
        
        if(v->is_const) const_declared.insert(v->name);
        else declared.insert(v->name);
        
        if(v->is_arr) {
            int esz = (v->type == "u8") ? 1 : 4;
            data << lb << ": .space " << (v->arr_size * esz) << "\n";
            return;
        }
        
        if(v->type == "string") {
            std::string sl = "str_" + std::to_string(scnt++);
            if(!v->init.empty()) {
                std::string str = v->init;
                if(str.size() >= 2 && str.front() == '"' && str.back() == '"')
                    str = str.substr(1, str.size() - 2);
                data << sl << ": .asciz \"" << str << "\"\n";
                data << lb << ": .quad " << sl << "\n";
            } else {
                data << lb << ": .quad 0\n";
            }
            return;
        }
        
        // Default initialization
        data << lb << ": .quad " << (v->init.empty() ? "0" : v->init) << "\n";
    }

    void gen_stmt(Node* n) {
        if(!n) return;
        switch(n->kind) {
            case NT::ASSIGN: gen_assign(static_cast<Assign*>(n)); break;
            case NT::DISPLAY: gen_display(static_cast<DisplayNode*>(n)); break;
            case NT::IF_STMT: gen_if(static_cast<IfNode*>(n)); break;
            case NT::LOOP: gen_loop(static_cast<LoopNode*>(n)); break;
            case NT::FOR: gen_for(static_cast<ForNode*>(n)); break;
            case NT::WHILE: gen_while(static_cast<WhileNode*>(n)); break;
            case NT::SWITCH_STMT: gen_switch(static_cast<SwitchNode*>(n)); break;
            // case NT::ASM_STMT: gen_asm(static_cast<AsmStmtNode*>(n)); break;  // TODO
            default: break;
        }
    }

    void gen_assign(Assign* a) {
        // Simple assignment for now
        load("x0", a->value);
        store("x0", a->target);
    }

    void gen_display(DisplayNode* d) {
        auto it = var_lbl.find(d->var);
        if(it == var_lbl.end()) return;
        
        // Write syscall
        if (macos_arm64) {
            code << "    mov x1, 1\n";  // stdout
            code << "    adrp x0, " << it->second << "@PAGE\n";
            code << "    add x0, x0, " << it->second << "@PAGEOFF\n";
            code << "    mov x2, #100\n";  // max length
            code << "    mov x16, #4\n";   // write syscall
            code << "    svc #0x80\n";
        } else {
            code << "    mov x0, #1\n";  // stdout
            code << "    adrp x1, " << it->second << "@PAGE\n";
            code << "    add x1, x1, " << it->second << "@PAGEOFF\n";
            code << "    mov x2, #100\n";
            code << "    mov x8, #64\n";  // write syscall
            code << "    svc #0\n";
        }
    }

    void gen_if(IfNode* n) {
        std::string L = lbl("if_skip");
        std::string Le = lbl("if_end");
        
        // Load and compare
        load("x0", n->left);
        if(is_num(n->right)) {
            code << "    mov x1, #" << n->right << "\n";
        } else {
            load("x1", n->right);
        }
        
        // Compare and branch
        code << "    cmp x0, x1\n";
        if(n->op == "==") code << "    b.ne " << L << "\n";
        else if(n->op == "!=") code << "    b.eq " << L << "\n";
        else if(n->op == "<") code << "    b.ge " << L << "\n";
        else if(n->op == ">") code << "    b.le " << L << "\n";
        else if(n->op == "<=") code << "    b.gt " << L << "\n";
        else if(n->op == ">=") code << "    b.lt " << L << "\n";
        
        for(auto& s : n->then_body) gen_stmt(s.get());
        
        if(!n->else_body.empty()) {
            code << "    b " << Le << "\n";
            code << L << ":\n";
            for(auto& s : n->else_body) gen_stmt(s.get());
            code << Le << ":\n";
        } else {
            code << L << ":\n";
        }
    }

    void gen_loop(LoopNode* l) {
        std::string ls = lbl("loop_start");
        std::string le = lbl("loop_end");
        loop_ends.push_back(le);
        
        code << ls << ":\n";
        for(auto& s : l->body) gen_stmt(s.get());
        
        code << "    b " << ls << "\n";
        code << le << ":\n";
        loop_ends.pop_back();
    }

    void gen_for(ForNode* f) {
        std::string fs = lbl("for_start");
        std::string fe = lbl("for_end");
        
        // Init
        load("x0", f->init_value);
        store("x0", f->init_var);
        
        code << fs << ":\n";
        
        // Condition
        auto it = var_lbl.find(f->init_var);
        code << "    ldr x0, [x29, #" << it->second << "@PAGEOFF]\n";
        if(is_num(f->cond_right)) {
            code << "    mov x1, #" << f->cond_right << "\n";
        }
        code << "    cmp x0, x1\n";
        code << "    b.ge " << fe << "\n";
        
        loop_ends.push_back(fe);
        for(auto& s : f->body) gen_stmt(s.get());
        loop_ends.pop_back();
        
        // Step
        code << "    ldr x0, [x29, #" << it->second << "@PAGEOFF]\n";
        code << "    add x0, x0, #1\n";
        code << "    str x0, [x29, #" << it->second << "@PAGEOFF]\n";
        
        code << "    b " << fs << "\n";
        code << fe << ":\n";
    }

    void gen_while(WhileNode* w) {
        std::string ws = lbl("while_start");
        std::string we = lbl("while_end");
        
        code << ws << ":\n";
        
        load("x0", w->left);
        if(is_num(w->right)) {
            code << "    mov x1, #" << w->right << "\n";
        } else {
            load("x1", w->right);
        }
        
        code << "    cmp x0, x1\n";
        if(w->op == "<") code << "    b.ge " << we << "\n";
        
        loop_ends.push_back(we);
        for(auto& s : w->body) gen_stmt(s.get());
        loop_ends.pop_back();
        
        code << "    b " << ws << "\n";
        code << we << ":\n";
    }

    void gen_switch(SwitchNode* s) {
        std::string end = lbl("switch_end");
        std::vector<std::string> labels;
        
        load("x0", s->value);
        
        for(size_t i = 0; i < s->cases.size(); i++) {
            std::string lbl = "case_" + std::to_string(i);
            labels.push_back(lbl);
            
            if(is_num(s->cases[i].first)) {
                code << "    cmp x0, #" << s->cases[i].first << "\n";
            }
            code << "    b.ne " << lbl << "_next\n";
            code << "    b " << lbl << "\n";
            code << lbl << "_next:\n";
        }
        
        if(!s->default_body.empty()) {
            code << "    b default_case\n";
        } else {
            code << "    b " << end << "\n";
        }
        
        for(size_t i = 0; i < s->cases.size(); i++) {
            code << labels[i] << ":\n";
            for(auto& stmt : s->cases[i].second) gen_stmt(stmt.get());
            code << "    b " << end << "\n";
        }
        
        if(!s->default_body.empty()) {
            code << "default_case:\n";
            for(auto& stmt : s->default_body) gen_stmt(stmt.get());
        }
        
        code << end << ":\n";
    }

    void gen_func(FuncDecl* f) {
        std::string nm = f->name;
        if(!nm.empty() && nm[0] == '#') nm = nm.substr(1);
        
        code << "\n" << nm << ":\n";
        code << "    stp x29, x30, [sp, #-16]!\n";
        code << "    mov x29, sp\n";
        
        gen_section(f->body.get());
        
        code << "    ldp x29, x30, [sp], #16\n";
        code << "    ret\n";
    }
};
