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

    // Expression AST node
    struct ExprNode {
        std::string op;       // "", "+", "-", "*", "/"
        std::string value;    // variable, number, etc.
        std::unique_ptr<ExprNode> left, right;
        ExprNode() {}
        ExprNode(std::string v) : value(v) {}
        ExprNode(std::string o, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r)
            : op(o), left(std::move(l)), right(std::move(r)) {}
    };
    
    std::unique_ptr<ExprNode> parse_primary() {
        // Handle & (address-of)
        if (at(TT::AMP)) {
            adv();
            if (!at(TT::IDENT)) {
                throw std::runtime_error("expected variable name after '&' at line " + std::to_string(cur().line));
            }
            std::string var = cur().val;
            adv();
            return std::make_unique<ExprNode>("&" + var);
        }

        // Handle * (dereference): *ptr
        if (at(TT::STAR)) {
            adv();
            if (!at(TT::IDENT)) {
                throw std::runtime_error("expected variable name after '*' at line " + std::to_string(cur().line));
            }
            std::string ptr = cur().val;
            adv();
            return std::make_unique<ExprNode>("*" + ptr);
        }

        // Handle logical NOT: !x
        if (at(TT::LOGIC_NOT)) {
            adv();
            auto operand = parse_primary();
            return std::make_unique<ExprNode>("!", std::move(operand), std::make_unique<ExprNode>("0"));
        }

        // Handle true/false literals
        if (at(TT::TRUE)) {
            adv();
            return std::make_unique<ExprNode>("1");
        }
        if (at(TT::FALSE)) {
            adv();
            return std::make_unique<ExprNode>("0");
        }

        // Handle array initialization: [1, 2, 3]
        if (at(TT::LBRACK)) {
            adv();  // consume '['
            std::string arr_init = "[";
            bool first = true;
            while (!at(TT::RBRACK) && !at(TT::EOF_T)) {
                if (!first) arr_init += ",";
                first = false;
                if (at(TT::NUMBER) || at(TT::HEX) || at(TT::STR_LIT)) {
                    arr_init += cur().val;
                    adv();
                } else if (at(TT::TRUE)) {
                    arr_init += "1";
                    adv();
                } else if (at(TT::FALSE)) {
                    arr_init += "0";
                    adv();
                } else if (at(TT::IDENT)) {
                    arr_init += cur().val;
                    adv();
                } else {
                    break;
                }
            }
            arr_init += "]";
            if (at(TT::RBRACK)) adv();  // consume ']'
            return std::make_unique<ExprNode>(arr_init);
        }

        // Handle parenthesized expressions
        if (at(TT::LPAREN)) {
            adv();  // consume '('
            auto expr = parse_additive();
            if (!at(TT::RPAREN)) {
                throw std::runtime_error("expected ')' at line " + std::to_string(cur().line));
            }
            adv();  // consume ')'
            return expr;
        }

        // Handle negative numbers (unary minus)
        if (at(TT::MINUS)) {
            adv();
            std::string val = cur().val;
            adv();
            // Check if it's a number
            bool is_number = !val.empty() && (isdigit(val[0]) || (val.size() > 1 && val[0] == '-' && isdigit(val[1])));
            if (is_number) {
                return std::make_unique<ExprNode>("-" + val);
            }
            return std::make_unique<ExprNode>("-" + val);
        }

        std::string v = cur().val;
        adv();
        return std::make_unique<ExprNode>(v);
    }
    
    // Parse multiplication and division (higher precedence)
    std::unique_ptr<ExprNode> parse_multiplicative() {
        auto left = parse_primary();
        while (at(TT::STAR) || at(TT::DIV)) {
            std::string op = cur().val;
            adv();
            auto right = parse_primary();
            left = std::make_unique<ExprNode>(op, std::move(left), std::move(right));
        }
        return left;
    }
    
    // Parse addition and subtraction (lower precedence)
    std::unique_ptr<ExprNode> parse_additive() {
        auto left = parse_multiplicative();
        while (at(TT::PLUS) || at(TT::MINUS)) {
            std::string op = cur().val;
            adv();
            auto right = parse_multiplicative();
            left = std::make_unique<ExprNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // Parse comparison operators (even lower precedence)
    std::unique_ptr<ExprNode> parse_comparison() {
        auto left = parse_additive();
        while (at(TT::EQEQ) || at(TT::NEQ) || at(TT::LT) || at(TT::GT) || at(TT::LTE) || at(TT::GTE)) {
            std::string op = cur().val;
            adv();
            auto right = parse_additive();
            left = std::make_unique<ExprNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // Parse logical AND (&&) - higher precedence than ||
    std::unique_ptr<ExprNode> parse_logic_and() {
        auto left = parse_comparison();
        while (at(TT::LOGIC_AND)) {
            std::string op = cur().val;
            adv();
            auto right = parse_comparison();
            left = std::make_unique<ExprNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // Parse logical OR (||) - lowest precedence
    std::unique_ptr<ExprNode> parse_logic_or() {
        auto left = parse_logic_and();
        while (at(TT::LOGIC_OR)) {
            std::string op = cur().val;
            adv();
            auto right = parse_logic_and();
            left = std::make_unique<ExprNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<ExprNode> parse_expression() {
        return parse_logic_or();
    }
    
    // Serialize expression AST to string for codegen
    // Format: "a+b*c" or "(a+b)*c" with proper parentheses
    std::string serialize_expr(ExprNode* node) {
        if (!node) return "";
        if (node->op.empty()) {
            return node->value;
        }
        std::string left = serialize_expr(node->left.get());
        std::string right = serialize_expr(node->right.get());
        return "(" + left + node->op + right + ")";
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
        
        // Parse pointer types: *i32, **i32, *struct, etc.
        std::string type_str = "";
        while(at(TT::STAR)) {
            type_str += "*";
            adv();
        }
        
        // Accept built-in types or struct names (IDENT)
        if(at(TT::I32)||at(TT::I64)||at(TT::U8)||at(TT::STR)||at(TT::PTR)||at(TT::BOOL)){
            type_str += cur().val;
            n->type = type_str;
            adv();
        } else if(at(TT::IDENT)){
            // Struct type or unknown type
            type_str += cur().val;
            n->type = type_str;
            adv();
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
            else if(at(TT::TOK_NULL))     { n->init="0"; adv(); }  // null initializer for pointers
            else if(at(TT::TRUE))     { n->init="1"; adv(); }  // true for bool
            else if(at(TT::FALSE))    { n->init="0"; adv(); }  // false for bool
            else if(at(TT::MINUS)) {
                // Negative number: var x: i32 = -42
                adv();
                if(at(TT::NUMBER)) {
                    n->init = "-" + cur().val;
                    adv();
                } else {
                    throw std::runtime_error("expected number after '-' at line " + std::to_string(cur().line));
                }
            }
            else if(at(TT::AMP)) {
                // Address-of initializer: var ptr: *i32 = &x
                adv();
                if(!at(TT::IDENT)) throw std::runtime_error("expected variable name after '&' at line " + std::to_string(cur().line));
                n->init = "&" + cur().val;
                adv();
            }
            else if(at(TT::STAR)) {
                // Dereference initializer: var y: i32 = *ptr
                adv();
                if(!at(TT::IDENT)) throw std::runtime_error("expected variable name after '*' at line " + std::to_string(cur().line));
                n->init = "*" + cur().val;
                adv();
            }
            else if(at(TT::LBRACK)) {
                // Array initializer: var arr: i32[5] = [1, 2, 3]
                n->init = "[";
                bool first = true;
                adv();  // consume '['
                while (!at(TT::RBRACK) && !at(TT::EOF_T)) {
                    if (!first) n->init += ",";
                    first = false;
                    if (at(TT::NUMBER) || at(TT::HEX) || at(TT::STR_LIT)) {
                        n->init += cur().val;
                        adv();
                    } else if (at(TT::TRUE)) {
                        n->init += "1";
                        adv();
                    } else if (at(TT::FALSE)) {
                        n->init += "0";
                        adv();
                    } else if (at(TT::IDENT)) {
                        n->init += cur().val;
                        adv();
                    } else {
                        break;
                    }
                }
                n->init += "]";
                if (at(TT::RBRACK)) adv();
            }
            else throw std::runtime_error("expected initializer at line "+std::to_string(cur().line));
        }
        if(is_const_decl && n->init.empty())
            throw std::runtime_error("const requires initializer at line "+std::to_string(cur().line));
        if(is_const_decl) const_vars.insert(n->name);
        return n;
    }

    NodePtr parse_stmt() {
        if (at(TT::FREE) || at(TT::DEALLOC)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<DeallocNode>(); n->ptr=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::ALLOC)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<AllocNode>(); n->size=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::DISPLAY)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<DisplayNode>(); n->var=cur().val; adv();
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::PRINTNUM)) {
            adv(); expect(TT::LBRACE,"expected '{'");
            auto n=std::make_unique<PrintNumNode>(); n->var=cur().val; adv();
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
        if (at(TT::WHILE)) {
            adv();
            auto n=std::make_unique<WhileNode>();
            n->left=cur().val; adv(); n->op=cur().val; adv(); n->right=cur().val; adv();
            expect(TT::LBRACE,"expected '{'");
            while(!at(TT::RBRACE)&&!at(TT::EOF_T)) { auto s=parse_stmt(); if(s) n->body.push_back(std::move(s)); }
            expect(TT::RBRACE,"expected '}'"); return n;
        }
        if (at(TT::FOR)) {
            adv();
            auto n = std::make_unique<ForNode>();
            // Syntax: for i = 0 to 10 { }
            n->init_var = cur().val;
            adv();
            expect(TT::EQ, "expected '='");
            n->init_value = cur().val;
            adv();
            expect(TT::TO, "expected 'to' - old for syntax with ';' is no longer supported");
            n->cond_left = n->init_var;
            n->cond_op = "<";
            n->cond_right = cur().val;
            adv();  // consume the limit value
            // Step is always i = (i + 1)
            n->step_var = n->init_var;
            n->step_value = "(" + n->init_var + "+1)";

            expect(TT::LBRACE, "expected '{'");
            while(!at(TT::RBRACE)&&!at(TT::EOF_T)) { auto s=parse_stmt(); if(s) n->body.push_back(std::move(s)); }
            expect(TT::RBRACE, "expected '}'");
            return n;
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
        if (at(TT::SWITCH)) {
            return parse_switch();
        }
        if (at(TT::STOP)) { adv(); return std::make_unique<BreakNode>(); }
        if (at(TT::CONTINUE)) { adv(); return std::make_unique<ContinueNode>(); }
        if (at(TT::RETURN)) {
            adv();  // consume 'return'
            auto n = std::make_unique<ReturnNode>();
            expect(TT::LBRACE, "expected '{'");
            n->value = cur().val;
            adv();
            expect(TT::RBRACE, "expected '}'");
            return n;
        }
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
        // Check for dereference assignment: *ptr = value
        if (at(TT::STAR)) {
            adv();
            if (!at(TT::IDENT)) {
                throw std::runtime_error("expected variable name after '*' at line " + std::to_string(cur().line));
            }
            auto n=std::make_unique<Assign>();
            n->target = "*" + cur().val;
            adv();
            expect(TT::EQ,"expected '='");
            // Parse full expression with nested support
            auto expr = parse_expression();
            // Convert expression AST to string for codegen
            std::string expr_str = "";
            if (expr) {
                // Simple case: just a value
                if (expr->op.empty()) {
                    expr_str = expr->value;
                } else {
                    expr_str = serialize_expr(expr.get());
                }
            }
            n->value = expr_str;
            return n;
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
            // Parse full expression with nested support
            auto expr = parse_expression();
            // Convert expression AST to string for codegen
            std::string expr_str = "";
            if (expr) {
                // Simple case: just a value
                if (expr->op.empty()) {
                    expr_str = expr->value;
                } else {
                    // Complex expression - need to generate temp variables
                    // For now, store as flattened string (will be handled by codegen)
                    expr_str = serialize_expr(expr.get());
                }
            }
            n->value = expr_str;
            return n;
        }
        if (!at(TT::SEC_CLOSE)&&!at(TT::RBRACE)&&!at(TT::EOF_T)) {
            warn("unexpected token '"+cur().val+"', skipping", cur().line); adv();
        }
        return nullptr;
    }

    std::unique_ptr<SectionNode> parse_section() {
        expect(TT::SEC_OPEN, "expected '<.de'");
        auto s = std::make_unique<SectionNode>();
        // Declarations and statements can be mixed freely
        while (!at(TT::SEC_CLOSE) && !at(TT::EOF_T)) {
            if (at(TT::VAR) || at(TT::CONST)) {
                s->decls.push_back(parse_decl());
            } else if (at(TT::STRUCT) || at(TT::ENUM)) {
                // Nested struct/enum definitions not allowed in sections
                throw std::runtime_error(
                    "struct/enum definitions are not allowed inside sections (at line " + std::to_string(cur().line) + ")");
            } else {
                // Parse statement
                auto st = parse_stmt();
                if (st) s->stmts.push_back(std::move(st));
            }
        }
        expect(TT::SEC_CLOSE, "expected '.>'");
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
            // Support pointer types: *i32, **i32, etc.
            while (at(TT::STAR)) {
                ftype += "*";
                adv();
            }
            if (at(TT::I32) || at(TT::I64) || at(TT::U8) || at(TT::STR) || at(TT::PTR) || at(TT::BOOL)) {
                ftype += cur().val;
                adv();
            } else if (at(TT::IDENT)) {
                // Struct type
                ftype += cur().val;
                adv();
            } else {
                throw std::runtime_error("expected type at line " + std::to_string(cur().line));
            }
            // Support array types: u8[256], i32[10], etc.
            if (at(TT::LBRACK)) {
                adv();  // consume '['
                if (!at(TT::NUMBER)) {
                    throw std::runtime_error("expected array size at line " + std::to_string(cur().line));
                }
                ftype += "[" + cur().val + "]";
                adv();  // consume number
                expect(TT::RBRACK, "expected ']'");
            }
            s->fields.push_back({fname, ftype});
        }
        expect(TT::RBRACE, "expected '}'");
        return s;
    }

    std::unique_ptr<EnumDecl> parse_enum() {
        expect(TT::ENUM, "expected 'enum'");
        auto e = std::make_unique<EnumDecl>();
        e->name = cur().val;
        expect(TT::IDENT, "expected enum name");
        expect(TT::LBRACE, "expected '{'");
        int val = 0;
        while (!at(TT::RBRACE) && !at(TT::EOF_T)) {
            std::string vname = cur().val;
            expect(TT::IDENT, "expected variant name");
            e->variants.push_back(vname);
            // Check for = value
            if (at(TT::EQ)) {
                adv();
                if (at(TT::NUMBER)) {
                    val = std::stoi(cur().val);
                    adv();
                }
            }
            val++;
            // Check for comma or end
            if (at(TT::COMMA)) {
                adv();
            }
        }
        expect(TT::RBRACE, "expected '}'");
        return e;
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
        expect(TT::FN, "expected 'fn'");
        auto n = std::make_unique<FuncDecl>();
        n->name = cur().val;
        adv();
        
        // Parse optional parameters: fn name(param1: i32, param2: string) { }
        if (at(TT::LPAREN)) {
            adv();  // consume '('
            while (!at(TT::RPAREN) && !at(TT::EOF_T)) {
                std::string param_name = cur().val;
                expect(TT::IDENT, "expected parameter name");
                expect(TT::COLON, "expected ':' after parameter name");
                std::string param_type;
                if (at(TT::I32) || at(TT::I64) || at(TT::U8) || at(TT::STR) || at(TT::PTR) || at(TT::BOOL)) {
                    param_type = cur().val;
                    adv();
                } else if (at(TT::IDENT)) {
                    // Struct type
                    param_type = cur().val;
                    adv();
                } else {
                    throw std::runtime_error("expected parameter type at line " + std::to_string(cur().line));
                }
                n->params.push_back({param_name, param_type});
                if (at(TT::COMMA)) {
                    adv();
                } else {
                    break;
                }
            }
            expect(TT::RPAREN, "expected ')' after parameters");
        }
        
        // fn name { <.de ... .> }
        expect(TT::LBRACE, "expected '{' after function name");
        n->body = parse_section();
        expect(TT::RBRACE, "expected '}'");
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

    std::unique_ptr<DriverDecl> parse_driver() {
        expect(TT::DRIVER_KEYWORD, "expected 'driver'");
        auto d = std::make_unique<DriverDecl>();
        d->name = cur().val;
        expect(TT::IDENT, "expected driver name");
        
        // Optional: { type = keyboard }
        if (at(TT::LBRACE)) {
            adv();  // consume '{'
            while (!at(TT::RBRACE) && !at(TT::EOF_T)) {
                if (at(TT::TYPE)) {
                    adv();  // consume 'type'
                    expect(TT::EQ, "expected '='");
                    d->type = "#" + cur().val;
                    adv();
                } else {
                    adv();
                }
            }
            expect(TT::RBRACE, "expected '}'");
        } else {
            // No type specified, use name as type
            d->type = "#" + d->name;
        }
        return d;
    }

public:
    explicit Parser(std::vector<Token> tokens) : tk(std::move(tokens)) {}

    std::unique_ptr<ProgramNode> parse(bool is_library = false) {
        auto p=std::make_unique<ProgramNode>();
        
        // Libraries don't require #Mainprogramm.start
        if (!is_library) {
            expect(TT::PROG_START,"file must begin with '#Mainprogramm.start'");
            // Parse directives and imports after #Mainprogramm.start
            while(at(TT::NO_RUNTIME)||at(TT::SAFE)||at(TT::DRIVER)||at(TT::IMPORT)) {
                if(at(TT::NO_RUNTIME)){p->no_runtime=true;adv();}
                if(at(TT::SAFE)){p->safe=true;adv();}
                if(at(TT::DRIVER)){p->no_runtime=true;adv();}
                if(at(TT::IMPORT)) {
                    adv();
                    expect(TT::LBRACE, "expected '{' after 'import'");
                    std::string libname = cur().val;
                    expect(TT::IDENT, "expected library name");
                    expect(TT::RBRACE, "expected '}' after library name");
                    p->imports.push_back(libname);
                    adv();
                }
            }
        } else {
            // For libraries, skip comments and whitespace until we find content
            while(!at(TT::EOF_T) && !at(TT::FN) && !at(TT::STRUCT) && !at(TT::IMPORT)) {
                adv();
            }
            // Parse imports in library
            while(at(TT::IMPORT)) {
                adv();
                expect(TT::LBRACE, "expected '{' after 'import'");
                std::string libname = cur().val;
                expect(TT::IDENT, "expected library name");
                expect(TT::RBRACE, "expected '}' after library name");
                p->imports.push_back(libname);
                adv();
            }
        }

        // Parse top-level declarations in any order
        while (!at(TT::SEC_OPEN) && !at(TT::PROG_END) && !at(TT::EOF_T)) {
            if (at(TT::ENUM)) {
                parse_enum();
            } else if (at(TT::DRIVER_KEYWORD)) {
                p->drivers.push_back(parse_driver());
            } else if (at(TT::EXTERN)) {
                auto ext = parse_extern();
                p->externs.push_back(std::unique_ptr<ExternDecl>(static_cast<ExternDecl*>(ext.release())));
            } else if (at(TT::STRUCT)) {
                p->structs.push_back(parse_struct());
            } else if (at(TT::INTERRUPT)) {
                p->interrupts.push_back(parse_interrupt());
            } else if (at(TT::FN)) {
                p->functions.push_back(parse_function());
            } else if (at(TT::INCLUDE)) {
                p->main_sec.push_back(parse_include());
            } else {
                adv();  // skip unknown token
            }
        }
        if(at(TT::SEC_OPEN))     p->main_sec.push_back(parse_section());
        if (!is_library) {
            expect(TT::PROG_END,"file must end with '#Mainprogramm.end'");
        }
        if(at(TT::DRIVER_STOP)) adv();  // Optional #DRIVER.stop
        return p;
    }

    NodePtr parse_extern() {
        expect(TT::EXTERN, "expected 'extern'");
        auto e = std::make_unique<ExternDecl>();
        e->name = cur().val;
        expect(TT::IDENT, "expected function name");
        // Optional: extern name from "library"
        if (at(TT::FROM)) {
            adv();
            if (at(TT::STR_LIT)) {
                e->library = cur().val;
                adv();
            }
        }
        return e;
    }

    NodePtr parse_include() {
        expect(TT::INCLUDE, "expected 'include'");
        auto i = std::make_unique<IncludeNode>();
        i->path = cur().val;
        if (at(TT::STR_LIT)) {
            i->path = cur().val;
            adv();
        } else {
            expect(TT::IDENT, "expected include path");
        }
        return i;
    }

    NodePtr parse_switch() {
        expect(TT::SWITCH, "expected 'switch'");
        auto s = std::make_unique<SwitchNode>();
        s->value = cur().val;
        adv();
        expect(TT::LBRACE, "expected '{'");
        
        while (!at(TT::RBRACE) && !at(TT::EOF_T)) {
            if (at(TT::CASE)) {
                adv();  // consume 'case'
                std::string case_val = cur().val;
                adv();
                expect(TT::COLON, "expected ':'");
                NodeList body;
                while (!at(TT::CASE) && !at(TT::DEFAULT) && !at(TT::RBRACE) && !at(TT::EOF_T)) {
                    auto stmt = parse_stmt();
                    if (stmt) body.push_back(std::move(stmt));
                }
                s->cases.push_back({case_val, std::move(body)});
            } else if (at(TT::DEFAULT)) {
                adv();  // consume 'default'
                expect(TT::COLON, "expected ':'");
                while (!at(TT::CASE) && !at(TT::RBRACE) && !at(TT::EOF_T)) {
                    auto stmt = parse_stmt();
                    if (stmt) s->default_body.push_back(std::move(stmt));
                }
            } else {
                adv();
            }
        }
        expect(TT::RBRACE, "expected '}'");
        return s;
    }
};
