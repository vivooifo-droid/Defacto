#pragma once
#include "defacto.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

class LLVMCodeGen {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    
    std::map<std::string, llvm::Value*> variables;
    std::map<std::string, llvm::Type*> struct_types;
    std::map<std::string, std::map<std::string, int>> struct_field_offsets;
    std::map<std::string, int> struct_sizes;
    std::vector<llvm::BasicBlock*> loop_ends;
    std::vector<llvm::BasicBlock*> loop_continues;
    
    llvm::Type* i32_type;
    llvm::Type* i64_type;
    llvm::Type* i8_type;
    llvm::Type* i1_type;
    llvm::Type* ptr_type;
    llvm::Type* void_type;
    
    bool use_gc = false;
    bool bare_metal = false;
    bool is_64bit = false;

public:
    LLVMCodeGen() : builder(context), use_gc(false), bare_metal(false), is_64bit(false) {
        // Initialize LLVM
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        
        // Create module
        module = std::make_unique<llvm::Module>("defacto_module", context);
        
        // Setup basic types
        i32_type = llvm::Type::getInt32Ty(context);
        i64_type = llvm::Type::getInt64Ty(context);
        i8_type = llvm::Type::getInt8Ty(context);
        i1_type = llvm::Type::getInt1Ty(context);
        ptr_type = llvm::Type::getPtrTy(context);
        void_type = llvm::Type::getVoidTy(context);
    }
    
    void set_gc(bool enable) { use_gc = enable; }
    void set_bare_metal(bool enable) { bare_metal = enable; }
    void set_64bit(bool enable) { is_64bit = enable; }
    
    llvm::Type* get_llvm_type(const std::string& type_name) {
        if (type_name == "i32") return i32_type;
        if (type_name == "i64") return i64_type;
        if (type_name == "u8") return i8_type;
        if (type_name == "bool") return i1_type;
        if (type_name == "string" || type_name == "pointer") return ptr_type;
        if (type_name.find('*') == 0) return ptr_type;
        
        // Check for struct type
        auto it = struct_types.find(type_name);
        if (it != struct_types.end()) return it->second;
        
        // Default to i32
        return i32_type;
    }
    
    int get_type_size(const std::string& type_name) {
        if (type_name == "i32") return 4;
        if (type_name == "i64") return 8;
        if (type_name == "u8") return 1;
        if (type_name == "bool") return 1;
        if (type_name == "string" || type_name == "pointer") return is_64bit ? 8 : 4;
        if (type_name.find('*') == 0) return is_64bit ? 8 : 4;
        
        auto it = struct_sizes.find(type_name);
        if (it != struct_sizes.end()) return it->second;
        
        return 4; // default
    }
    
    void gen_struct(StructDecl* s) {
        int offset = 0;
        std::vector<llvm::Type*> field_types;
        
        for (auto& f : s->fields) {
            std::string ftype = f.second;
            int fsize = 4;
            
            // Check for array type
            size_t bracket_pos = ftype.find('[');
            if (bracket_pos != std::string::npos) {
                std::string base_type = ftype.substr(0, bracket_pos);
                size_t close_bracket = ftype.find(']', bracket_pos);
                int arr_size = std::stoi(ftype.substr(bracket_pos + 1, close_bracket - bracket_pos - 1));
                
                if (base_type == "u8") fsize = 1 * arr_size;
                else if (base_type == "i32") fsize = 4 * arr_size;
                else if (base_type == "i64" || base_type == "string" || base_type == "pointer") 
                    fsize = 8 * arr_size;
                else fsize = 4 * arr_size;
                
                field_types.push_back(llvm::ArrayType::get(get_llvm_type(base_type), arr_size));
            } else {
                fsize = get_type_size(ftype);
                field_types.push_back(get_llvm_type(ftype));
            }
            
            struct_field_offsets[s->name][f.first] = offset;
            offset += fsize;
        }
        
        struct_sizes[s->name] = offset;
        struct_types[s->name] = llvm::StructType::create(context, field_types, s->name);
    }
    
    llvm::Value* load_value(llvm::Value* ptr, const std::string& type_name) {
        llvm::Type* ty = get_llvm_type(type_name);
        if (ty->isPointerTy()) {
            return builder.CreateLoad(ptr_type, ptr);
        } else if (ty == i32_type) {
            return builder.CreateLoad(i32_type, ptr);
        } else if (ty == i64_type) {
            return builder.CreateLoad(i64_type, ptr);
        } else if (ty == i8_type) {
            return builder.CreateLoad(i8_type, ptr);
        }
        return builder.CreateLoad(ty, ptr);
    }
    
    void store_value(llvm::Value* value, llvm::Value* ptr) {
        builder.CreateStore(value, ptr);
    }
    
    llvm::Value* parse_expression(const std::string& expr) {
        std::string s = expr;
        
        // Remove outer parentheses
        while (s.size() >= 2 && s[0] == '(' && s[s.size()-1] == ')') {
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
            if (match) s = s.substr(1, s.size() - 2);
            else break;
        }
        
        // Find main operator (lowest precedence first)
        size_t op_pos = std::string::npos;
        char op_char = 0;
        int paren_depth = 0;
        
        // Look for + or -
        for (int i = (int)s.size() - 1; i >= 0; i--) {
            char c = s[i];
            if (c == ')') paren_depth++;
            else if (c == '(') paren_depth--;
            else if (paren_depth == 0 && (c == '+' || c == '-')) {
                if (i == 0 || (s[i-1] != '(' && s[i-1] != '+' && s[i-1] != '-' && 
                               s[i-1] != '*' && s[i-1] != '/' && s[i-1] != '&' && 
                               s[i-1] != '|' && s[i-1] != '^')) {
                    op_pos = i;
                    op_char = c;
                    break;
                }
            }
        }
        
        // Look for * or /
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
        
        // Look for bitwise operators
        if (op_pos == std::string::npos) {
            paren_depth = 0;
            for (int i = (int)s.size() - 1; i >= 0; i--) {
                char c = s[i];
                if (c == ')') paren_depth++;
                else if (c == '(') paren_depth--;
                else if (paren_depth == 0 && c == '&') {
                    op_pos = i;
                    op_char = c;
                    break;
                }
            }
        }
        
        if (op_pos == std::string::npos) {
            paren_depth = 0;
            for (int i = (int)s.size() - 1; i >= 0; i--) {
                char c = s[i];
                if (c == ')') paren_depth++;
                else if (c == '(') paren_depth--;
                else if (paren_depth == 0 && (c == '|' || c == '^')) {
                    op_pos = i;
                    op_char = c;
                    break;
                }
            }
        }
        
        // Look for shift operators
        if (op_pos == std::string::npos) {
            for (size_t i = 0; i + 1 < s.size(); i++) {
                if ((s[i] == '<' && s[i+1] == '<') || (s[i] == '>' && s[i+1] == '>')) {
                    op_pos = i;
                    op_char = s[i];
                    break;
                }
            }
        }
        
        // No operator: it's a value
        if (op_pos == std::string::npos) {
            return parse_primary(s);
        }
        
        std::string left_str = s.substr(0, op_pos);
        std::string right_str = s.substr(op_pos + 1);
        
        llvm::Value* left = parse_expression(left_str);
        llvm::Value* right = parse_expression(right_str);
        
        if (!left || !right) return nullptr;
        
        switch (op_char) {
            case '+': return builder.CreateAdd(left, right);
            case '-': return builder.CreateSub(left, right);
            case '*': return builder.CreateMul(left, right);
            case '/': return builder.CreateSDiv(left, right);
            case '&': return builder.CreateAnd(left, right);
            case '|': return builder.CreateOr(left, right);
            case '^': return builder.CreateXor(left, right);
            case '<': 
                if (s[op_pos+1] == '<') return builder.CreateShl(left, right);
                return builder.CreateICmpSLT(left, right);
            case '>':
                if (s[op_pos+1] == '>') return builder.CreateAShr(left, right);
                return builder.CreateICmpSGT(left, right);
        }
        
        return left;
    }
    
    llvm::Value* parse_primary(const std::string& s) {
        if (s.empty()) return nullptr;
        
        // Number
        if (isdigit(s[0]) || (s[0] == '-' && s.size() > 1 && isdigit(s[1]))) {
            if (s.find('.') != std::string::npos) {
                // Float literal - treat as i64 for now
                return llvm::ConstantInt::get(i64_type, std::stoll(s));
            }
            return llvm::ConstantInt::get(i32_type, std::stoll(s));
        }
        
        // Hex number
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            return llvm::ConstantInt::get(i32_type, std::stoll(s.substr(2), nullptr, 16));
        }
        
        // Address-of: &var
        if (s[0] == '&') {
            std::string varname = s.substr(1);
            auto it = variables.find(varname);
            if (it != variables.end()) return it->second;
            return nullptr;
        }
        
        // Dereference: *ptr
        if (s[0] == '*') {
            std::string ptrname = s.substr(1);
            auto it = variables.find(ptrname);
            if (it != variables.end()) {
                return builder.CreateLoad(ptr_type, it->second);
            }
            return nullptr;
        }
        
        // Variable reference
        auto it = variables.find(s);
        if (it != variables.end()) {
            return load_value(it->second, "i32"); // TODO: get actual type
        }
        
        return nullptr;
    }
    
    void gen_var(VarDecl* v) {
        llvm::Type* ty = get_llvm_type(v->type);
        llvm::Value* alloc;
        
        if (v->is_arr) {
            int arr_size = v->arr_size;
            int elem_size = (v->type == "u8") ? 1 : 4;
            llvm::Type* elem_ty = get_llvm_type(v->type);
            alloc = builder.CreateAlloca(llvm::ArrayType::get(elem_ty, arr_size), nullptr, "var_" + v->name);
        } else {
            alloc = builder.CreateAlloca(ty, nullptr, "var_" + v->name);
        }
        
        variables[v->name] = alloc;
        
        // Initialize if needed
        if (!v->init.empty()) {
            llvm::Value* init_val = parse_expression(v->init);
            if (init_val) {
                store_value(init_val, alloc);
            }
        }
    }
    
    void gen_display(DisplayNode* d) {
        // For now, just create a placeholder - will be implemented with printf
        auto printf_func = module->getFunction("printf");
        if (!printf_func) {
            llvm::FunctionType* printf_type = llvm::FunctionType::get(
                i32_type, {ptr_type}, true);
            printf_func = llvm::Function::Create(printf_type, 
                llvm::Function::ExternalLinkage, "printf", module.get());
        }
        
        auto it = variables.find(d->var);
        if (it == variables.end()) return;
        
        llvm::Value* var_ptr = it->second;
        llvm::Value* var_val = load_value(var_ptr, "i32");
        
        // Create format string
        llvm::Constant* fmt_str = llvm::ConstantDataArray::getString(context, "%d\n");
        llvm::GlobalVariable* fmt_global = new llvm::GlobalVariable(
            *module, fmt_str->getType(), true, llvm::GlobalValue::PrivateLinkage, 
            fmt_str, ".str");
        
        llvm::Value* fmt_ptr = builder.CreateGEP(
            fmt_global->getValueType(), fmt_global, 
            {llvm::ConstantInt::get(i32_type, 0), llvm::ConstantInt::get(i32_type, 0)});
        
        builder.CreateCall(printf_func, {fmt_ptr, var_val});
    }
    
    std::string generate(std::vector<NodePtr>& nodes, bool is_64bit_mode) {
        set_64bit(is_64bit_mode);
        
        // Create main function
        llvm::FunctionType* main_type = llvm::FunctionType::get(i32_type, false);
        llvm::Function* main_func = llvm::Function::Create(
            main_type, llvm::Function::ExternalLinkage, "main", module.get());
        
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", main_func);
        builder.SetInsertPoint(entry);
        
        // Process all nodes
        for (auto& node : nodes) {
            if (auto* var = dynamic_cast<VarDecl*>(node.get())) {
                gen_var(var);
            } else if (auto* display = dynamic_cast<DisplayNode*>(node.get())) {
                gen_display(display);
            }
            // TODO: Handle other node types
        }
        
        // Return 0
        builder.CreateRet(llvm::ConstantInt::get(i32_type, 0));
        
        // Verify module
        llvm::verifyModule(*module, &llvm::errs());
        
        // Optimize
        llvm::legacy::FunctionPassManager fpm(module.get());
        fpm.add(llvm::createInstructionCombiningPass());
        fpm.add(llvm::createReassociatePass());
        fpm.add(llvm::createGVNPass());
        fpm.add(llvm::createCFGSimplificationPass());
        fpm.doInitialization();
        
        for (auto& func : *module) {
            fpm.run(func);
        }
        
        // Output LLVM IR
        std::string ir;
        llvm::raw_string_ostream os(ir);
        module->print(os, nullptr);
        
        return ir;
    }
    
    bool write_bitcode(const std::string& filename) {
        std::error_code EC;
        llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
        
        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message() << "\n";
            return false;
        }
        
        llvm::WriteBitcodeToFile(*module, dest);
        dest.flush();
        return true;
    }
};
