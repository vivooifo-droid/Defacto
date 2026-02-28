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
    PROG_START, PROG_END, NO_RUNTIME, SAFE, INTERRUPT, DRIVER, DRIVER_STOP,
    SEC_OPEN, SEC_CLOSE, STATIC_PL, DRV_OPEN, DRV_CLOSE,
    VAR, CONST, CONST_DRIVER, FUNCTION, FN, DRIVER_KEYWORD, CALL, LOOP, IF, ELSE, STOP, DISPLAY, PRINTNUM, FREE, COLOR, READKEY, READCHAR, PUTCHAR, CLEAR, REBOOT,
    IMPORT, INCLUDE, FROM, RETURN, WHILE, FOR, TO, ENUM, TRY, CATCH, SWITCH, CASE, DEFAULT,
    STRUCT, CONTINUE, EXTERN,
    MOV, REG_STATIC, REG_STOP,
    I32, I64, U8, STR, PTR, BOOL,
    TRUE, FALSE,
    REGISTER, IDENT, NUMBER, STR_LIT, HEX,
    EQ, EQEQ, NEQ, LT, GT, LTE, GTE, PLUS, MINUS, MUL, DIV, LSHIFT,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACK, RBRACK,
    COLON, SEMICOLON, COMMA, DOT,
    DRV_FUNC_ASSIGN, DRV_CALL, DRV_CALL_NOT,
    AMP, STAR, TOK_NULL, ALLOC, DEALLOC,
    LOGIC_AND, LOGIC_OR, LOGIC_NOT,
    ARROW, TYPE,
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
    ASSIGN, LOOP, WHILE, FOR, IF_STMT, REG_OP, DISPLAY, PRINTNUM, FREE, BREAK, INTERRUPT, COLOR, READKEY, READCHAR, PUTCHAR, CLEAR, REBOOT,
    RETURN, CONTINUE_STMT,
    IMPORT, INCLUDE,
    DRIVER_SECTION, CONST_DRIVER_DECL, DRV_FUNC_ASSIGN, DRV_CALL, DRIVER_DECL, EXTERN_DECL,
    STRUCT_DECL, STRUCT_FIELD_ACCESS, ENUM_DECL,
    PTR_ADDR, PTR_DEREF, ALLOC_NODE, DEALLOC_NODE,
    ARRAY_INIT, LOGIC_EXPR,
    SWITCH_STMT, CASE_LABEL
};

struct Node { NT kind; virtual ~Node() = default; };
using NodePtr = std::unique_ptr<Node>;
using NodeList = std::vector<NodePtr>;

struct SectionNode : Node {
    NodeList decls, stmts;
    SectionNode() { kind = NT::SECTION; }
};

// Struct support - must be before ProgramNode
struct StructDecl : Node {
    std::string name;
    std::vector<std::pair<std::string, std::string>> fields;  // field name -> type
    StructDecl() { kind = NT::STRUCT_DECL; }
};

// Switch/Case support
struct SwitchNode : Node {
    std::string value;  // value to switch on
    std::vector<std::pair<std::string, NodeList>> cases;  // case value -> body
    NodeList default_body;  // default case body
    SwitchNode() { kind = NT::SWITCH_STMT; }
};

struct ExternDecl : Node {
    std::string name;
    std::string library;  // optional library name
    ExternDecl() { kind = NT::EXTERN_DECL; }
};

struct IncludeNode : Node {
    std::string path;
    IncludeNode() { kind = NT::INCLUDE; }
};

// Driver support - must be before ProgramNode
struct DriverDecl : Node {
    std::string name;
    std::string type;  // keyboard, mouse, volume
    DriverDecl() { kind = NT::DRIVER_DECL; }
};

struct ProgramNode : Node {
    bool no_runtime = false, safe = false;
    NodeList interrupts, functions, main_sec;
    std::vector<std::unique_ptr<StructDecl>> structs;
    std::vector<std::unique_ptr<DriverDecl>> drivers;  // New driver declarations
    std::vector<std::unique_ptr<ExternDecl>> externs;  // Extern function declarations
    std::vector<std::string> imports;  // List of imported libraries
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
    std::vector<std::pair<std::string, std::string>> params;  // param name -> type
    std::string return_type;
    std::unique_ptr<SectionNode> body;
    FuncDecl() { kind = NT::FUNC_DECL; }
};

struct FuncCall : Node {
    std::string name;
    std::vector<std::string> args;  // Function arguments
    FuncCall() { kind = NT::FUNC_CALL; }
};

struct ContinueNode : Node {
    ContinueNode() { kind = NT::CONTINUE_STMT; }
};

struct EnumDecl : Node {
    std::string name;
    std::vector<std::string> variants;
    EnumDecl() { kind = NT::ENUM_DECL; }
};

struct ArrayInit : Node {
    std::vector<std::string> values;
    ArrayInit() { kind = NT::ARRAY_INIT; }
};

struct ReturnNode : Node {
    std::string value;
    ReturnNode() { kind = NT::RETURN; }
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

struct WhileNode : Node {
    std::string left, op, right;
    NodeList body;
    WhileNode() { kind = NT::WHILE; }
};

struct ForNode : Node {
    std::string init_var, init_value;
    std::string cond_left, cond_op, cond_right;
    std::string step_var, step_value;
    NodeList body;
    ForNode() { kind = NT::FOR; }
};

struct IfNode : Node {
    std::string left, op, right;
    NodeList then_body, else_body;
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

struct PrintNumNode : Node {
    std::string var;
    PrintNumNode() { kind = NT::PRINTNUM; }
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

// Driver support
struct DriverSectionNode : Node {
    NodeList decls, stmts;
    std::string driver_name;
    std::string driver_type;  // keyboard, mouse, volume
    DriverSectionNode() { kind = NT::DRIVER_SECTION; }
};

struct ConstDriverDecl : Node {
    std::string name;
    ConstDriverDecl() { kind = NT::CONST_DRIVER_DECL; }
};

struct DriverFuncAssign : Node {
    std::string driver_name;
    std::string driver_type;
    DriverFuncAssign() { kind = NT::DRV_FUNC_ASSIGN; }
};

struct DriverCall : Node {
    std::string driver_target;
    std::string builtin_name;
    bool use_builtin = true;
    DriverCall() { kind = NT::DRV_CALL; }
};

struct StructFieldAccess : Node {
    std::string struct_var;
    std::string field_name;
    StructFieldAccess() { kind = NT::STRUCT_FIELD_ACCESS; }
};

// Pointer support
struct PtrAddrNode : Node {
    std::string var;  // Variable to take address of
    PtrAddrNode() { kind = NT::PTR_ADDR; }
};

struct PtrDerefNode : Node {
    std::string ptr;  // Pointer to dereference
    PtrDerefNode() { kind = NT::PTR_DEREF; }
};

struct AllocNode : Node {
    std::string size;  // Size in bytes
    AllocNode() { kind = NT::ALLOC_NODE; }
};

struct DeallocNode : Node {
    std::string ptr;  // Pointer to free
    DeallocNode() { kind = NT::DEALLOC_NODE; }
};
