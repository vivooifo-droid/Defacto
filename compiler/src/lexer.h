#pragma once
#include "defacto.h"
#include <cctype>
#include <stdexcept>

class Lexer {
    const std::string& src;
    size_t pos = 0;
    int    line = 1, col = 0;

    char cur()           const { return pos < src.size() ? src[pos] : 0; }
    char pk(int n=1)     const { return pos+n < src.size() ? src[pos+n] : 0; }
    void adv() {
        if (pos < src.size()) {
            col++;
            if (src[pos++] == '\n') { line++; col = 0; }
        }
    }
    void skip_ws()  { while (cur()==' '||cur()=='\t'||cur()=='\r') adv(); }
    void skip_cmt() { while (cur()!='\n'&&cur()) adv(); }

    std::string read_ident() {
        std::string s;
        while (isalnum(cur())||cur()=='_'||cur()=='.') { s+=cur(); adv(); }
        return s;
    }
    std::string read_num() {
        std::string s;
        while (isdigit(cur())) { s+=cur(); adv(); }
        return s;
    }
    std::string read_str() {
        adv(); std::string s;
        while (cur()!='"'&&cur()) {
            if (cur()=='\\') { adv();
                switch(cur()) {
                    case 'n': s+='\n'; break;
                    case 't': s+='\t'; break;
                    default:  s+=cur();
                }
            } else s+=cur();
            adv();
        }
        if (cur()=='"') adv();
        return s;
    }

    TT kw(const std::string& w) {
        if(w=="var")           return TT::VAR;
        if(w=="const")         return TT::CONST;
        if(w=="Const.driver")  return TT::CONST_DRIVER;
        if(w=="function")      return TT::FUNCTION;
        if(w=="call")          return TT::DRV_CALL;
        if(w=="loop")          return TT::LOOP;
        if(w=="if")            return TT::IF;
        if(w=="stop")          return TT::STOP;
        if(w=="display")       return TT::DISPLAY;
        if(w=="free")          return TT::FREE;
        if(w=="color")         return TT::COLOR;
        if(w=="readkey")       return TT::READKEY;
        if(w=="readchar")      return TT::READCHAR;
        if(w=="putchar")       return TT::PUTCHAR;
        if(w=="clear")         return TT::CLEAR;
        if(w=="reboot")        return TT::REBOOT;
        if(w=="i32")           return TT::I32;
        if(w=="i64")           return TT::I64;
        if(w=="u8")            return TT::U8;
        if(w=="string")        return TT::STR;
        if(w=="pointer")       return TT::PTR;
        if(w=="keyboard")      return TT::IDENT;  // Driver type
        if(w=="mouse")         return TT::IDENT;  // Driver type
        if(w=="volume")        return TT::IDENT;  // Driver type
        return TT::IDENT;
    }

public:
    explicit Lexer(const std::string& s) : src(s) {}

    std::vector<Token> tokenize() {
        std::vector<Token> out;
        while (pos < src.size()) {
            skip_ws();
            if (!cur()) break;
            if (cur()=='\n') { adv(); continue; }
            if (cur()=='/'&&pk()=='/') { skip_cmt(); continue; }

            int l=line, c=col;

            if (cur()=='#') {
                adv();
                if (cur()=='0'&&(pk()=='x'||pk()=='X')) {
                    std::string h="0x"; adv(); adv();
                    while (isxdigit(cur())) { h+=cur(); adv(); }
                    out.emplace_back(TT::HEX, h, l, c); continue;
                }
                std::string w = read_ident();
                if      (w=="DRIVER")        out.emplace_back(TT::DRIVER, "#DRIVER", l, c);
                else if (w=="DRIVER.stop")   out.emplace_back(TT::DRIVER_STOP, "#DRIVER.stop", l, c);
                else if (w=="keyboard")      out.emplace_back(TT::IDENT, "#keyboard", l, c);
                else if (w=="mouse")         out.emplace_back(TT::IDENT, "#mouse", l, c);
                else if (w=="volume")        out.emplace_back(TT::IDENT, "#volume", l, c);
                else if (w=="Mainprogramm.start") out.emplace_back(TT::PROG_START, w, l, c);
                else if (w=="Mainprogramm.end")   out.emplace_back(TT::PROG_END,   w, l, c);
                else if (w=="NO_RUNTIME")         out.emplace_back(TT::NO_RUNTIME, w, l, c);
                else if (w=="SAFE")               out.emplace_back(TT::SAFE,       w, l, c);
                else if (w=="INTERRUPT")          out.emplace_back(TT::INTERRUPT,  w, l, c);
                else if (w=="MOV")                out.emplace_back(TT::MOV,        w, l, c);
                else if (w=="STATIC")             out.emplace_back(TT::REG_STATIC, w, l, c);
                else if (w=="STOP")               out.emplace_back(TT::REG_STOP,   w, l, c);
                else if (!w.empty()&&w[0]=='R'&&w.size()>1&&isdigit(w[1]))
                    out.emplace_back(TT::REGISTER, "#"+w, l, c);
                else
                    out.emplace_back(TT::IDENT, "#"+w, l, c);
                continue;
            }

            // Debug: print current character
            // std::cerr << "DEBUG: cur='" << cur() << "' pk(1)='" << pk(1) << "' pk(2)='" << pk(2) << "' pk(3)='" << pk(3) << "' line=" << line << "\n";
            
            if (cur()=='<'&&pk(1)=='d'&&pk(2)=='r'&&pk(3)=='v'&&pk(4)=='.') {
                adv();adv();adv();adv();adv();
                out.emplace_back(TT::DRV_OPEN,"<drv.",l,c); continue;
            }
            if (cur()=='.'&&pk(1)=='d'&&pk(2)=='r'&&pk(3)=='>') {
                adv();adv();adv();adv();
                out.emplace_back(TT::DRV_CLOSE,".dr>",l,c); continue;
            }
            if (cur()=='<'&&pk(1)=='.'&&pk(2)=='d'&&pk(3)=='e') {
                adv();adv();adv();adv();
                out.emplace_back(TT::SEC_OPEN,"<.de",l,c); continue;
            }
            if (cur()=='.'&&pk()=='>') {
                adv();adv();
                out.emplace_back(TT::SEC_CLOSE,".>",l,c); continue;
            }
            if (cur()=='"') { out.emplace_back(TT::STR_LIT, read_str(), l, c); continue; }
            if (isdigit(cur())) { out.emplace_back(TT::NUMBER, read_num(), l, c); continue; }
            if (isalpha(cur())||cur()=='_') {
                std::string w = read_ident();
                if (w=="static.pl"&&cur()=='>') { adv(); out.emplace_back(TT::STATIC_PL,"static.pl>",l,c); }
                else out.emplace_back(kw(w), w, l, c);
                continue;
            }
            char ch=cur();
            if (ch=='='&&pk()=='=') { adv();adv(); out.emplace_back(TT::EQEQ, "==",l,c); }
            else if(ch=='<'&&pk()=='<') { adv();adv(); out.emplace_back(TT::DRV_FUNC_ASSIGN, "<<",l,c); }
            else if(ch=='-'&&pk()=='>') { adv();adv(); out.emplace_back(TT::LSHIFT, "->",l,c); }
            else if(ch=='>'&&pk()=='>') { adv();adv(); out.emplace_back(TT::RBRACK, ">>",l,c); }
            else if(ch=='='){adv();out.emplace_back(TT::EQ,    "=",l,c);}
            else if(ch=='+'){adv();out.emplace_back(TT::PLUS,  "+",l,c);}
            else if(ch=='-'){adv();out.emplace_back(TT::MINUS, "-",l,c);}
            else if(ch=='*'){adv();out.emplace_back(TT::MUL,   "*",l,c);}
            else if(ch=='/'){adv();out.emplace_back(TT::DIV,   "/",l,c);}
            else if(ch=='('){adv();out.emplace_back(TT::LPAREN,"(",l,c);}
            else if(ch==')'){adv();out.emplace_back(TT::RPAREN,")",l,c);}
            else if(ch=='{'){adv();out.emplace_back(TT::LBRACE,"{",l,c);}
            else if(ch=='}'){adv();out.emplace_back(TT::RBRACE,"}",l,c);}
            else if(ch=='['){adv();out.emplace_back(TT::LBRACK,"[",l,c);}
            else if(ch==']'){adv();out.emplace_back(TT::RBRACK,"]",l,c);}
            else if(ch==':'){adv();out.emplace_back(TT::COLON, ":",l,c);}
            else if(ch==','){adv();out.emplace_back(TT::COMMA, ",",l,c);}
            else { err("unknown character '"+std::string(1,ch)+"'", line); adv(); }
        }
        out.emplace_back(TT::EOF_T,"",line,col);
        return out;
    }
};
