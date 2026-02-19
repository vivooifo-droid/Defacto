#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <iostream>


#define RED    "\033[1;31m"
#define YELLOW "\033[1;33m"
#define RESET  "\033[0m"

inline void err(const std::string& msg, int line = -1) {
    std::cerr << RED << "error";
    if (line > 0) std::cerr << "[" << line << "]";
    std::cerr << ": " << msg << RESET << "\n";
}
inline void warn(const std::string& msg, int line = -1) {
    std::cerr << YELLOW << "warning";
    if (line > 0) std::cerr << "[" << line << "]";
    std::cerr << ": " << msg << RESET << "\n";
}


enum class TT {
    PROG_START, PROG_END, NO_RUNTIME, SAFE, INTERRUPT,
    SEC_OPEN, SEC_CLOSE, STATIC_PL,
    VAR, CONST, FUNCTION, CALL, LOOP, IF, STOP, DISPLAY, FREE, COLOR, READKEY, READCHAR, PUTCHAR, CLEAR, REBOOT,
    MOV, REG_STATIC, REG_STOP,
    I32, I64, U8, STR, PTR,
    REGISTER, IDENT, NUMBER, STR_LIT, HEX,
    EQ, EQEQ, PLUS, MINUS, MUL, DIV,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACK, RBRACK,
    COLON, COMMA,
    EOF_T
};

struct Token {
    TT          type;
    std::string val;
    int         line, col;
    Token(TT t, std::string v, int l=0, int c=0)
        : type(t), val(std::move(v)), line(l), col(c) {}
};

enum class NT {
    PROGRAM, SECTION, VAR_DECL, FUNC_DECL, FUNC_CALL,
    ASSIGN, LOOP, IF_STMT, REG_OP, DISPLAY, FREE, BREAK, INTERRUPT, COLOR, READKEY, READCHAR, PUTCHAR, CLEAR, REBOOT
};

struct Node { NT kind; virtual ~Node() = default; };
using NodePtr = std::unique_ptr<Node>;
using NodeList = std::vector<NodePtr>;

struct SectionNode : Node {
    NodeList decls, stmts;
    SectionNode() { kind = NT::SECTION; }
};

struct ProgramNode : Node {
    bool no_runtime = false, safe = false;
    NodeList interrupts, functions, main_sec;
    ProgramNode() { kind = NT::PROGRAM; }
};

struct VarDecl : Node {
    std::string name, type, init;
    int arr_size = 0;
    bool is_arr  = false;
    bool is_const = false;
    VarDecl() { kind = NT::VAR_DECL; }
};

struct FuncDecl : Node {
    std::string name;
    std::unique_ptr<SectionNode> body;
    FuncDecl() { kind = NT::FUNC_DECL; }
};

struct FuncCall : Node {
    std::string name;
    FuncCall() { kind = NT::FUNC_CALL; }
};

struct Assign : Node {
    std::string target, value, idx;
    bool is_reg = false, is_arr = false;
    Assign() { kind = NT::ASSIGN; }
};

struct LoopNode : Node {
    NodeList body;
    LoopNode() { kind = NT::LOOP; }
};

struct IfNode : Node {
    std::string left, op, right;
    NodeList then_body;
    IfNode() { kind = NT::IF_STMT; }
};

struct RegOp : Node {
    std::string op, target, source;
    RegOp() { kind = NT::REG_OP; }
};

struct DisplayNode : Node {
    std::string var;
    DisplayNode() { kind = NT::DISPLAY; }
};

struct FreeNode : Node {
    std::string var;
    FreeNode() { kind = NT::FREE; }
};

struct ColorNode : Node {
    std::string value;
    ColorNode() { kind = NT::COLOR; }
};

struct ReadKeyNode : Node {
    std::string var;
    ReadKeyNode() { kind = NT::READKEY; }
};

struct ReadCharNode : Node {
    std::string var;
    ReadCharNode() { kind = NT::READCHAR; }
};

struct PutCharNode : Node {
    std::string value;
    PutCharNode() { kind = NT::PUTCHAR; }
};

struct ClearNode : Node {
    ClearNode() { kind = NT::CLEAR; }
};

struct RebootNode : Node {
    RebootNode() { kind = NT::REBOOT; }
};

struct BreakNode : Node {
    BreakNode() { kind = NT::BREAK; }
};

struct InterruptNode : Node {
    int num = 0;
    std::string func;
    InterruptNode() { kind = NT::INTERRUPT; }
};
