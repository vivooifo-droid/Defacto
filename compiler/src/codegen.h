#pragma once
#include "defacto.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

class CodeGen {
    std::ostringstream code;
    std::ostringstream data;

    std::map<std::string,std::string> var_lbl;
    std::map<std::string,bool> var_is_ptr;
    std::set<std::string> declared, freed, const_declared, driver_constants;
    std::vector<std::string> loop_ends;
    int lcnt = 0, scnt = 0;
    bool bare_metal = true;
    bool macos_terminal = false;

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
        auto it=var_lbl.find(dst);
        if(it==var_lbl.end()) throw std::runtime_error("undefined variable '"+dst+"'");
        if(macos_terminal && var_is_ptr[dst]) code<<"    mov qword ["<<addr(it->second)<<"], "<<src_reg<<"\n";
        else code<<"    mov dword ["<<addr(it->second)<<"], "<<src_reg<<"\n";
    }

    void expr(const std::string& dst, const std::string& e){
        std::string s=e;
        for(char c:{'(',')',' '}) s.erase(std::remove(s.begin(),s.end(),c),s.end());
        size_t op=std::string::npos;
        for(size_t i=1;i<s.size();i++) if(s[i]=='+'||s[i]=='-'||s[i]=='*'||s[i]=='/'){op=i;break;}
        if(op==std::string::npos){load(dst,s);return;}
        std::string L=s.substr(0,op), R=s.substr(op+1); char o=s[op];
        load(dst,L);
        auto rhs=[&]()->std::string{
            if(is_reg(R)) return reg(R);
            if(is_num(R)||is_hex(R)) return R;
            auto it=var_lbl.find(R);
            if(it==var_lbl.end()) throw std::runtime_error("undefined variable '"+R+"'");
            return "dword ["+addr(it->second)+"]";
        };
        if(o=='+') code<<"    add "<<dst<<", "<<rhs()<<"\n";
        else if(o=='-') code<<"    sub "<<dst<<", "<<rhs()<<"\n";
        else if(o=='*') code<<"    imul "<<dst<<", "<<rhs()<<"\n";
        else if(o=='/'){
            code<<"    push edx\n    xor edx, edx\n    mov ecx, "<<rhs()<<"\n";
            if(dst!="eax") code<<"    mov eax, "<<dst<<"\n";
            code<<"    idiv ecx\n";
            if(dst!="eax") code<<"    mov "<<dst<<", eax\n";
            code<<"    pop edx\n";
        }
    }

    void gen_var(VarDecl* v){
        std::string lb="var_"+v->name;
        var_lbl[v->name]=lb;
        var_is_ptr[v->name]=(v->type=="string"||v->type=="pointer");
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
        if(macos_terminal && v->type=="pointer"){
            data<<"    align 8\n";
            data<<"    "<<lb<<": dq "<<(v->init.empty()?"0":v->init)<<"\n";
        } else {
            data<<"    "<<lb<<": dd "<<(v->init.empty()?"0":v->init)<<"\n";
        }
    }

    void gen_display(DisplayNode* d){
            auto it=var_lbl.find(d->var);
            if(it==var_lbl.end()){warn("display: unknown variable '"+d->var+"'");return;}
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
        auto it=var_lbl.find(k->var);
        if(it==var_lbl.end()) throw std::runtime_error("readchar: undefined variable '"+k->var+"'");
        if(!bare_metal){
            code<<"    mov dword ["<<addr(it->second)<<"], 0\n";
            return;
        }
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

    void gen_if(IfNode* n){
        std::string L=lbl("if_skip");
        load("eax",n->left);
        if(is_num(n->right))      code<<"    cmp eax, "<<n->right<<"\n";
        else if(is_reg(n->right)) code<<"    cmp eax, "<<reg(n->right)<<"\n";
        else{auto it=var_lbl.find(n->right);code<<"    cmp eax, dword ["<<addr(it->second)<<"]\n";}
        if(n->op=="==")      code<<"    jne "<<L<<"\n";
        else if(n->op=="!=") code<<"    je  "<<L<<"\n";
        else if(n->op=="<")  code<<"    jge "<<L<<"\n";
        else if(n->op==">")  code<<"    jle "<<L<<"\n";
        for(auto& s:n->then_body) gen_stmt(s.get());
        code<<L<<":\n";
    }

    void gen_stmt(Node* n){
        if(!n) return;
        switch(n->kind){
            case NT::ASSIGN:   gen_assign(static_cast<Assign*>(n)); break;
            case NT::REG_OP:   {auto r=static_cast<RegOp*>(n); if(r->op=="MOV") load(reg(r->target),r->source); break;}
            case NT::LOOP:     gen_loop(static_cast<LoopNode*>(n)); break;
            case NT::IF_STMT:  gen_if(static_cast<IfNode*>(n)); break;
            case NT::DISPLAY:  gen_display(static_cast<DisplayNode*>(n)); break;
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
            case NT::FUNC_CALL:{std::string nm=static_cast<FuncCall*>(n)->name;
                               if(!nm.empty()&&nm[0]=='#') nm=nm.substr(1);
                               code<<"    call "<<nm<<"\n"; break;}
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
            default: break;
        }
    }

    void gen_section(SectionNode* s){
        for(auto& d:s->decls) gen_var(static_cast<VarDecl*>(d.get()));
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
        if (driver_type == "keyboard") {
            code<<"\n__defacto_drv_keyboard:\n";
            code<<"    ; Built-in keyboard driver\n";
            code<<"    pushad\n";
            code<<"    call _init_keyboard\n";
            code<<"    popad\n";
            code<<"    ret\n";
        } else if (driver_type == "mouse") {
            code<<"\n__defacto_drv_mouse:\n";
            code<<"    ; Built-in mouse driver\n";
            code<<"    pushad\n";
            code<<"    call _init_mouse\n";
            code<<"    popad\n";
            code<<"    ret\n";
        } else if (driver_type == "volume") {
            code<<"\n__defacto_drv_volume:\n";
            code<<"    ; Built-in volume driver (PC speaker)\n";
            code<<"    pushad\n";
            code<<"    call _init_speaker\n";
            code<<"    popad\n";
            code<<"    ret\n";
        }
    }

    void gen_func(FuncDecl* f){
        std::string nm=f->name;
        if(!nm.empty()&&nm[0]=='#') nm=nm.substr(1);
        code<<"\n"<<nm<<":\n    push ebp\n    mov ebp, esp\n";
        gen_section(f->body.get());
        code<<"    mov esp, ebp\n    pop ebp\n    ret\n";
    }

    void check_mem(){
        bool ok=true;
        for(auto& v:declared)
            if(!freed.count(v) && !driver_constants.count(v)){
                err("Memory Abandonment: '"+v+"' never freed");
                ok=false;
            }
        if(!ok) throw std::runtime_error("memory abandonment errors");
    }

public:
    void set_mode(bool bm, bool macos=false){ bare_metal=bm; macos_terminal=macos; }

    void emit(ProgramNode* prog, const std::string& out_path){
        code<<"global _start\n";
        if(macos_terminal) code<<"section .text\n";
        code<<"_start:\n";
        
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
                code<<"\n    mov eax, 1\n    xor ebx, ebx\n    int 0x80\n";
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
        if(macos_terminal) f<<"section .data\n";
        if(bare_metal){
            f<<"__defacto_cursor: dd 0\n";
            f<<"__defacto_attr: db 15\n";
        }
        f<<data.str()<<"\n";
        f.close();
        std::cout<<"asm: "<<out_path<<"\n";
    }
};
