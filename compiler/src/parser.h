#pragma once
#include "defacto.h"
#include <stdexcept>

class Parser {
    std::vector<Token> tk;
    size_t pos = 0;
    std::set<std::string> const_vars;

    Token& cur()        { return tk[pos < tk.size() ? pos : tk.size()-1]; }
    void   adv()        { if (pos < tk.size()) pos++; }
    bool   at(TT t)     { return cur().type == t; }

    void expect(TT t, const std::string& msg) {
        if (!at(t)) throw std::runtime_error(msg+" (got '"+cur().val+"' at line "+std::to_string(cur().line)+")");
        adv();
    }

    std::string rhs() {
        if (at(TT::LPAREN)) {
            std::string e; int d=0;
            while (!at(TT::EOF_T)) {
                if (at(TT::LPAREN)) d++;
                if (at(TT::RPAREN)) { d--; e+=cur().val; adv(); if(!d) break; continue; }
                e+=cur().val; adv();
            }
            return e;
        }
        std::string v=cur().val; adv(); return v;
    }

    NodePtr parse_decl() {
        bool is_const_decl = false;
        if(at(TT::CONST)) {
            is_const_decl = true;
            adv();
        } else {
            expect(TT::VAR,"expected 'var'");
        }

        auto n=std::make_unique<VarDecl>();
        n->is_const = is_const_decl;
        if(!at(TT::IDENT)) throw std::runtime_error("expected variable name at line "+std::to_string(cur().line));
        n->name=cur().val; adv();
        expect(TT::COLON,"expected ':' after variable name at line "+std::to_string(cur().line));
        // Accept built-in types or struct names (IDENT)
        if(at(TT::I32)||at(TT::I64)||at(TT::U8)||at(TT::STR)||at(TT::PTR)){
            n->type=cur().val; adv();
        } else if(at(TT::IDENT)){
            // Struct type or unknown type
            n->type=cur().val; adv();
        } else {
            throw std::runtime_error("expected type at line "+std::to_string(cur().line));
        }
        if(at(TT::LBRACK)) {
            if(is_const_decl) throw std::runtime_error("const arrays are not supported at line "+std::to_string(cur().line));
            adv();
            if(!at(TT::NUMBER)) throw std::runtime_error("expected array size at line "+std::to_string(cur().line));
            n->arr_size=std::stoi(cur().val); n->is_arr=true; adv();
            expect(TT::RBRACK,"expected ']'");
        }
        if(at(TT::EQ)) {
            adv();
            if(at(TT::STR_LIT))      { n->init="\""+cur().val+"\""; adv(); }
            else if(at(TT::NUMBER))   { n->init=cur().val; adv(); }
            else if(at(TT::HEX))      { n->init=cur().val; adv(); }
            else throw std::runtime_error("expected initializer at line "+std::to_string(cur().line));
        }
        if(is_const_decl && n->init.empty())
            throw std::runtime_error("const requires initializer at line "+std::to_string(cur().line));
        if(is_const_decl) const_vars.insert(n->name);
        return n;
    }

    NodePtr parse_stmt() {
        if (at(TT::FREE)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<FreeNode>(); n->var=cur().val; adv();
            if(const_vars.count(n->var))
                throw std::runtime_error("cannot free const '"+n->var+"' at line "+std::to_string(cur().line));
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::DISPLAY)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<DisplayNode>(); n->var=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::COLOR)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<ColorNode>(); n->value=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::READKEY)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<ReadKeyNode>(); n->var=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::READCHAR)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<ReadCharNode>(); n->var=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::PUTCHAR)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<PutCharNode>(); n->value=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::CLEAR)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<ClearNode>();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::REBOOT)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<RebootNode>();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::CALL)) {
            adv();
            auto n=std::make_unique<FuncCall>(); n->name=cur().val; adv(); return n;
        }
        if (at(TT::LOOP)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<LoopNode>();
            while(!at(TT::RBRACE)&&!at(TT::EOF_T)) { auto s=parse_stmt(); if(s) n->body.push_back(std::move(s)); }
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::IF)) {
            adv();
            auto n=std::make_unique<IfNode>();
            n->left=cur().val; adv(); n->op=cur().val; adv(); n->right=cur().val; adv();
            expect(TT::LBRACE,"expected '{'");
            while(!at(TT::RBRACE)&&!at(TT::EOF_T)) { auto s=parse_stmt(); if(s) n->then_body.push_back(std::move(s)); }
            expect(TT::RBRACE,"expected '}'");
            // Check for else block
            if (at(TT::ELSE)) {
                adv();  // consume 'else'
                expect(TT::LBRACE,"expected '{' after 'else'");
                while(!at(TT::RBRACE)&&!at(TT::EOF_T)) { auto s=parse_stmt(); if(s) n->else_body.push_back(std::move(s)); }
                expect(TT::RBRACE,"expected '}'");
            }
            return n;
        }
        if (at(TT::STOP)) { adv(); return std::make_unique<BreakNode>(); }
        if (at(TT::MOV)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<RegOp>(); n->op="MOV";
            n->target=cur().val; adv();
            expect(TT::COMMA,"expected ','");
            n->source=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::REG_STATIC)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<RegOp>(); n->op="STATIC"; n->target=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::IDENT)||at(TT::REGISTER)) {
            auto n=std::make_unique<Assign>();
            std::string target_name = cur().val;
            adv();
            // Check for struct field access: struct.field
            if (at(TT::DOT)) {
                adv();  // consume '.'
                if (at(TT::IDENT)) {
                    n->target = target_name + "." + cur().val;
                    n->is_arr = true;  // Use is_arr flag to indicate struct field access
                    adv();
                } else {
                    throw std::runtime_error("expected field name after '.' at line " + std::to_string(cur().line));
                }
            } else {
                n->target = target_name;
            }
            if (n->target.size()>=2&&n->target[0]=='#'&&n->target[1]=='R') n->is_reg=true;
            if(!n->is_reg && const_vars.count(n->target))
                throw std::runtime_error("cannot assign to const '"+n->target+"' at line "+std::to_string(cur().line));
            if (at(TT::LBRACK)) { n->is_arr=true; adv(); n->idx=cur().val; adv(); expect(TT::RBRACK,"expected ']'"); }
            expect(TT::EQ,"expected '='");
            n->value=rhs(); return n;
        }
        if (!at(TT::SEC_CLOSE)&&!at(TT::RBRACE)&&!at(TT::EOF_T)) {
            warn("unexpected token '"+cur().val+"', skipping", cur().line); adv();
        }
        return nullptr;
    }

    std::unique_ptr<SectionNode> parse_section() {
        expect(TT::SEC_OPEN,"expected '<.de'");
        auto s=std::make_unique<SectionNode>();
        while(!at(TT::STATIC_PL)&&!at(TT::EOF_T)) {
            if(at(TT::VAR)||at(TT::CONST)) s->decls.push_back(parse_decl());
            else throw std::runtime_error(
                "only 'var' and 'const' allowed before 'static.pl>' (got '"+cur().val+"' at line "+std::to_string(cur().line)+")");
        }
        expect(TT::STATIC_PL,"expected 'static.pl>'");
        while(!at(TT::SEC_CLOSE)&&!at(TT::EOF_T)) { auto st=parse_stmt(); if(st) s->stmts.push_back(std::move(st)); }
        expect(TT::SEC_CLOSE,"expected '.>'");
        return s;
    }

    std::unique_ptr<StructDecl> parse_struct() {
        expect(TT::STRUCT, "expected 'struct'");
        auto s = std::make_unique<StructDecl>();
        s->name = cur().val;
        expect(TT::IDENT, "expected struct name");
        expect(TT::LBRACE, "expected '{'");
        while (!at(TT::RBRACE) && !at(TT::EOF_T)) {
            // Parse field: name: type
            std::string fname = cur().val;
            expect(TT::IDENT, "expected field name");
            expect(TT::COLON, "expected ':'");
            std::string ftype;
            if (at(TT::I32) || at(TT::I64) || at(TT::U8) || at(TT::STR) || at(TT::PTR)) {
                ftype = cur().val;
                adv();
            } else {
                throw std::runtime_error("expected type at line " + std::to_string(cur().line));
            }
            s->fields.push_back({fname, ftype});
        }
        expect(TT::RBRACE, "expected '}'");
        return s;
    }

    std::unique_ptr<DriverSectionNode> parse_driver_section() {
        expect(TT::DRV_OPEN, "expected '<drv.'");
        auto s=std::make_unique<DriverSectionNode>();

        // Parse Const.driver declaration
        if (at(TT::CONST_DRIVER)) {
            adv();
            expect(TT::EQ, "expected '='");
            auto cd = std::make_unique<ConstDriverDecl>();
            cd->name = cur().val;
            adv();
            s->driver_name = cd->name;
            s->decls.push_back(std::move(cd));
        }

        // Parse driver function assignment: name <<func = keyboard>>
        if (at(TT::IDENT)) {
            std::string name = cur().val;
            adv();

            // Check for << (DRV_FUNC_ASSIGN)
            if (at(TT::DRV_FUNC_ASSIGN)) {
                adv();  // <<
                std::string func_keyword = cur().val;  // Read BEFORE expect
                expect(TT::IDENT, "expected 'func'");  // This does adv(), now at '='
                expect(TT::EQ, "expected '='");
                
                // Parse driver type (keyboard, mouse, volume)
                std::string driver_type = cur().val;
                if (driver_type[0] != '#') {
                    driver_type = "#" + driver_type;
                }
                adv();
                
                // Skip >> if present
                if (at(TT::RBRACK)) {
                    adv();
                }

                auto dfa = std::make_unique<DriverFuncAssign>();
                dfa->driver_name = name;
                dfa->driver_type = driver_type;
                s->driver_type = driver_type;
                s->stmts.push_back(std::move(dfa));
            }
        }

        expect(TT::DRV_CLOSE, "expected '.dr>'");
        return s;
    }

    NodePtr parse_function() {
        expect(TT::FUNCTION,"expected 'function'");
        expect(TT::EQEQ,"expected '=='");
        auto n=std::make_unique<FuncDecl>(); n->name=cur().val; adv();
        expect(TT::LBRACE,"expected '{'");
        n->body=parse_section();
        expect(TT::RBRACE,"expected '}'");
        return n;
    }

    NodePtr parse_interrupt() {
        expect(TT::INTERRUPT,"expected '#INTERRUPT'");
        expect(TT::LBRACE,"expected '{'");
        auto n=std::make_unique<InterruptNode>(); n->num=std::stoi(cur().val); adv();
        expect(TT::RBRACE,"expected '}'");
        expect(TT::EQEQ,"expected '=='");
        n->func=cur().val; adv(); return n;
    }

public:
    explicit Parser(std::vector<Token> tokens) : tk(std::move(tokens)) {}

    std::unique_ptr<ProgramNode> parse() {
        auto p=std::make_unique<ProgramNode>();
        expect(TT::PROG_START,"file must begin with '#Mainprogramm.start'");
        while(at(TT::NO_RUNTIME)||at(TT::SAFE)||at(TT::DRIVER)) {
            if(at(TT::NO_RUNTIME)){p->no_runtime=true;adv();}
            if(at(TT::SAFE)){p->safe=true;adv();}
            if(at(TT::DRIVER)){p->no_runtime=true;adv();}  // DRIVER implies NO_RUNTIME
        }
        while(at(TT::STRUCT))      p->structs.push_back(parse_struct());
        while(at(TT::INTERRUPT)) p->interrupts.push_back(parse_interrupt());
        while(at(TT::FUNCTION))  p->functions.push_back(parse_function());
        if(at(TT::DRV_OPEN))     p->main_sec.push_back(parse_driver_section());
        if(at(TT::SEC_OPEN))     p->main_sec.push_back(parse_section());
        expect(TT::PROG_END,"file must end with '#Mainprogramm.end'");
        if(at(TT::DRIVER_STOP)) adv();  // Optional #DRIVER.stop
        return p;
    }
};
