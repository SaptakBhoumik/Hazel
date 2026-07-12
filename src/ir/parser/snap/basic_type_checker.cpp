#include "ir/parser/parser.hpp"
#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <iostream>

namespace Hazel {
namespace Snap{
namespace IR {
void Parser::basic_typecheck(){
    auto type_table = construct_type_table();
    auto functions = this->ast->get_items();
    std::vector<FunctionPtr> new_functions;
    std::unordered_map<std::string, TypeExprPtr> func_symbol_table;
    for(auto& func : functions){
        auto reduced_return_type = reduce_type_expr(func->get_return_type(), type_table);
        std::vector<std::pair<Token, TypeExprPtr>> reduced_params;
        std::vector<TypeExprPtr> reduced_param_types;
        for(auto& [param_name, param_type] : func->get_params()){
            auto reduced_param_type = reduce_type_expr(param_type, type_table);
            reduced_params.push_back({param_name, reduced_param_type});
            reduced_param_types.push_back(reduced_param_type);
        }
        auto new_func_type = std::make_shared<FuncTypeExpr>(func->get_token(), reduced_param_types, reduced_return_type);
        std::string func_name = func->get_name().value;
        if(func_symbol_table.find(func_name) != func_symbol_table.end()){
            error(func->get_name(), "Duplicate function definition", "Function name: " + func_name);
        }
        func_symbol_table[func_name] = new_func_type;
        if(func->get_body().empty()){
            new_functions.push_back(std::make_shared<Function>(func->get_token(), func->get_name(), reduced_params, reduced_return_type, func->get_body(), func->get_debug_info(), false));
        }
        else{
            
        }
    }
}
bool Parser::self_reference_type(TypeExprPtr type, std::string name) const{
    switch(type->get_kind()){
        case TypeExprKind::NamedTypeExpr:{
            auto named_type = std::dynamic_pointer_cast<NamedTypeExpr>(type);
            if(named_type->get_name().value == name){
                return true;
            }
            return false;
        }
        case TypeExprKind::IntTypeExpr:
        case TypeExprKind::DecimalTypeExpr:
        case TypeExprKind::StringTypeExpr:
        case TypeExprKind::VoidTypeExpr:
        case TypeExprKind::PtrTypeExpr:{
            return false;
        }
        case TypeExprKind::ArrayTypeExpr:{
            auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(type);
            return self_reference_type(array_type->get_basetype(), name);
        }
        case TypeExprKind::StructTypeExpr:{
            auto struct_type = std::dynamic_pointer_cast<StructTypeExpr>(type);
            for(auto& field : struct_type->get_fields()){
                if(self_reference_type(field, name)){
                    return true;
                }
            }
            return false;
        }
        case TypeExprKind::FuncTypeExpr:{
            auto func_type = std::dynamic_pointer_cast<FuncTypeExpr>(type);
            for(auto& param_type : func_type->get_param_types()){
                if(self_reference_type(param_type, name)){
                    return true;
                }
            }
            return self_reference_type(func_type->get_return_type(), name);
        }
        case TypeExprKind::LabelTypeExpr:{
            auto label_type = std::dynamic_pointer_cast<LabelTypeExpr>(type);
            for(auto& param_type : label_type->get_param_types()){
                if(self_reference_type(param_type, name)){
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}
std::unordered_map<std::string, TypeExprPtr> Parser::construct_type_table() const{
    std::unordered_map<std::string, TypeExprPtr> type_table;
    for(auto& [name_tok, type_expr] : this->global_typedef){
        auto it = type_table.find(name_tok.value);
        if(it != type_table.end()){
            error(name_tok, "Duplicate type definition", "Type name: " + name_tok.value);
        }
        if(self_reference_type(type_expr, name_tok.value)){
            error(name_tok, "Self-referencing type definition", "Type name: " + name_tok.value);
        }
        TypeExprPtr reduced_type = reduce_type_expr(type_expr, type_table);
        type_table[name_tok.value] = reduced_type;
    }
    return type_table;
}
TypeExprPtr Parser::reduce_type_expr(TypeExprPtr type, std::unordered_map<std::string, TypeExprPtr> type_table) const{
    switch(type->get_kind()){
        case TypeExprKind::NamedTypeExpr:{
            auto named_type = std::dynamic_pointer_cast<NamedTypeExpr>(type);
            auto it = type_table.find(named_type->get_name().value);
            if(it == type_table.end()){
                error(named_type->get_token(), "Named type not found in type table", "Type name: " + named_type->get_name().value);
            }
            return it->second;
        }
        case TypeExprKind::IntTypeExpr:
        case TypeExprKind::DecimalTypeExpr:
        case TypeExprKind::StringTypeExpr:
        case TypeExprKind::VoidTypeExpr:
        case TypeExprKind::PtrTypeExpr:{
            return type;
        }
        case TypeExprKind::ArrayTypeExpr:{
            auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(type);
            TypeExprPtr reduced_base_type = reduce_type_expr(array_type->get_basetype(), type_table);
            return std::make_shared<ArrayTypeExpr>(array_type->get_token(), reduced_base_type);
        }
        case TypeExprKind::StructTypeExpr:{
            auto struct_type = std::dynamic_pointer_cast<StructTypeExpr>(type);
            std::vector<TypeExprPtr> reduced_fields;
            for(auto& field : struct_type->get_fields()){
                reduced_fields.push_back(reduce_type_expr(field, type_table));
            }
            return std::make_shared<StructTypeExpr>(struct_type->get_token(), reduced_fields, struct_type->is_packed());
        }
        case TypeExprKind::FuncTypeExpr:{
            auto func_type = std::dynamic_pointer_cast<FuncTypeExpr>(type);
            std::vector<TypeExprPtr> reduced_param_types;
            for(auto& param_type : func_type->get_param_types()){
                reduced_param_types.push_back(reduce_type_expr(param_type, type_table));
            }
            TypeExprPtr reduced_return_type = reduce_type_expr(func_type->get_return_type(), type_table);
            return std::make_shared<FuncTypeExpr>(func_type->get_token(), reduced_param_types, reduced_return_type);
        }
        case TypeExprKind::LabelTypeExpr:{
            auto label_type = std::dynamic_pointer_cast<LabelTypeExpr>(type);
            std::vector<TypeExprPtr> reduced_param_types;
            for(auto& param_type : label_type->get_param_types()){
                reduced_param_types.push_back(reduce_type_expr(param_type, type_table));
            }
            return std::make_shared<LabelTypeExpr>(label_type->get_token(), reduced_param_types);
        }
        //Dont add default. This will make sure that if a new type is added, we will get a compile warning here and we will have to handle it here as well. 
    }
    return nullptr;//This should never happen. Just to avoid compiler warning
}
LiteralExprPtr Parser::reduce_literal_expr(LiteralExprPtr literal, std::string current_func, std::unordered_map<std::string, TypeExprPtr> type_table, 
                                           std::unordered_map<std::string, TypeExprPtr> func_symbol_table, std::unordered_map<std::string, TypeExprPtr> label_symbol_table,
                                           std::unordered_map<std::string, TypeExprPtr> local_var_table, std::unordered_map<std::string, TypeExprPtr> label_param_table) const{
    auto literal_type = literal->get_type();
    auto reduced_type = reduce_type_expr(literal_type, type_table);
    literal->set_type(reduced_type);
    switch(literal->get_kind()){
        case LiteralKind::ZeroInitLiteralExpr:
        case LiteralKind::PoisonLiteralExpr:{
            return literal;
        }
        case LiteralKind::NamedLiteralExpr:{
            auto it = label_param_table.find(literal->get_token().value);
            if(it != label_param_table.end()){
                if(!TypeUtils::is_type_equal(reduced_type, it->second)){
                    error(literal->get_token(), "Type mismatch for label parameter", "Expected type: " + it->second->to_string() + ", but got: " + reduced_type->to_string());
                }
                return literal;
            }
            it = local_var_table.find(literal->get_token().value);
            if(it != local_var_table.end()){
                if(!TypeUtils::is_type_equal(reduced_type, it->second)){    
                    error(literal->get_token(), "Type mismatch for local variable", "Expected type: " + it->second->to_string() + ", but got: " + reduced_type->to_string());
                }
                return literal;
            }
            error(literal->get_token(), "Named literal not found in local variable or label parameter table", "Named literal: " + literal->get_token().value);
        }
        case LiteralKind::NumLiteralExpr:{
            if(reduced_type->get_kind() != TypeExprKind::IntTypeExpr && reduced_type->get_kind() != TypeExprKind::DecimalTypeExpr){
                error(literal->get_token(), "Type mismatch for numeric literal", "Expected type: int or decimal, but got: " + reduced_type->to_string());
            }
            return literal;
        }
        case LiteralKind::StringLiteralExpr:{
            if(reduced_type->get_kind() != TypeExprKind::StringTypeExpr){
                error(literal->get_token(), "Type mismatch for string literal", "Expected type: string, but got: " + reduced_type->to_string());
            }
            return literal;
        }
        case LiteralKind::ArrayLiteralExpr:{
            if(reduced_type->get_kind() != TypeExprKind::ArrayTypeExpr){
                error(literal->get_token(), "Type mismatch for array literal", "Expected type: array, but got: " + reduced_type->to_string());
            }
            auto array_literal = std::dynamic_pointer_cast<ArrayLiteralExpr>(literal);
            auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(reduced_type);
            auto elements = array_literal->get_elements();
            for(size_t i=0;i<elements.size();i++){
                elements[i]->set_type(array_type->get_basetype());
                reduce_literal_expr(elements[i], current_func, type_table, func_symbol_table, label_symbol_table, local_var_table, label_param_table);
            }
            return std::make_shared<ArrayLiteralExpr>(array_literal->get_token(), elements, array_type);
        }
        case LiteralKind::StructLiteralExpr:{
            if(reduced_type->get_kind() != TypeExprKind::StructTypeExpr){
                error(literal->get_token(), "Type mismatch for struct literal", "Expected type: struct, but got: " + reduced_type->to_string());
            }
            auto struct_literal = std::dynamic_pointer_cast<StructLiteralExpr>(literal);
            auto struct_type = std::dynamic_pointer_cast<StructTypeExpr>(reduced_type);
            if(struct_literal->get_fields().size() != struct_type->get_fields().size()){
                error(literal->get_token(), "Field count mismatch for struct literal", "Expected field count: " + std::to_string(struct_type->get_fields().size()) + ", but got: " + std::to_string(struct_literal->get_fields().size()));
            }
            if(struct_literal->is_packed() != struct_type->is_packed()){
                error(literal->get_token(), "Packed attribute mismatch for struct literal", "Expected packed: " + std::string(struct_type->is_packed() ? "true" : "false") + ", but got: " + std::string(struct_literal->is_packed() ? "true" : "false"));
            }
            auto type_fields = struct_type->get_fields();
            auto value_fields = struct_literal->get_fields();
            for(size_t i=0;i<value_fields.size();i++){
                value_fields[i]->set_type(type_fields[i]);
                reduce_literal_expr(value_fields[i], current_func, type_table, func_symbol_table, label_symbol_table, local_var_table, label_param_table);
            }
            return std::make_shared<StructLiteralExpr>(struct_literal->get_token(), value_fields, struct_type->is_packed(), struct_type);
        }
        case LiteralKind::LabelLiteralExpr:{
            if(reduced_type->get_kind() != TypeExprKind::LabelTypeExpr){
                error(literal->get_token(), "Type mismatch for label literal", "Expected type: label, but got: " + reduced_type->to_string());
            }
            auto label_literal = std::dynamic_pointer_cast<LabelLiteralExpr>(literal);
            auto label_type = std::dynamic_pointer_cast<LabelTypeExpr>(reduced_type);
            if(label_literal->get_args().size() != label_type->get_param_types().size()){
                error(literal->get_token(), "Argument count mismatch for label literal", "Expected argument count: " + std::to_string(label_type->get_param_types().size()) + ", but got: " + std::to_string(label_literal->get_args().size()));
            }
            auto it = label_symbol_table.find(label_literal->get_name().value);
            if(it == label_symbol_table.end()){
                error(label_literal->get_name(), "Label not found in label symbol table", "Label name: " + label_literal->get_name().value);
            }
            auto type_params = label_type->get_param_types();
            auto value_args = label_literal->get_args();
            for(size_t i=0;i<value_args.size();i++){
                value_args[i]->set_type(type_params[i]);
                reduce_literal_expr(value_args[i], current_func, type_table, func_symbol_table, label_symbol_table, local_var_table, label_param_table);
            }
            return std::make_shared<LabelLiteralExpr>(label_literal->get_token(), value_args, label_type);
        }
        case LiteralKind::FuncLiteralExpr:{
            if(reduced_type->get_kind() != TypeExprKind::FuncTypeExpr){
                error(literal->get_token(), "Type mismatch for function literal", "Expected type: function, but got: " + reduced_type->to_string());
            }
            auto func_literal = std::dynamic_pointer_cast<FuncLiteralExpr>(literal);
            auto func_type = std::dynamic_pointer_cast<FuncTypeExpr>(reduced_type);
            if(func_literal->get_args().size() != func_type->get_param_types().size()){
                error(literal->get_token(), "Argument count mismatch for function literal", "Expected argument count: " + std::to_string(func_type->get_param_types().size()) + ", but got: " + std::to_string(func_literal->get_args().size()));
            }
            auto it = func_symbol_table.find(func_literal->get_name().value);
            if(it == func_symbol_table.end()){
                error(func_literal->get_name(), "Function not found in function symbol table", "Function name: " + func_literal->get_name().value);
            }
            if(func_literal->get_name().value == current_func){
                error(func_literal->get_name(), "Recursive function call is not allowed", "Function name: " + func_literal->get_name().value);
            }
            auto type_params = func_type->get_param_types();
            auto value_args = func_literal->get_args();
            for(size_t i=0;i<value_args.size();i++){
                value_args[i]->set_type(type_params[i]);
                reduce_literal_expr(value_args[i], current_func, type_table, func_symbol_table, label_symbol_table, local_var_table, label_param_table);
            }
            return std::make_shared<FuncLiteralExpr>(func_literal->get_token(), value_args, func_type);
        }
    }
    return nullptr;//This should never happen. Just to avoid compiler warning
}
}
}
}