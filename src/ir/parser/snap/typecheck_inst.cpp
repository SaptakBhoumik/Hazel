#include "ir/parser/parser.hpp"
#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <iostream>

namespace Hazel {
namespace Snap{
namespace IR {
void Parser::typecheck_inst(InstructionStmtPtr stmt, TypeExprPtr curr_func_type) const{
    std::string inst_name = stmt->get_instruction().value;
    auto params = stmt->get_params();
    auto dest = stmt->get_name();
    if(inst_name == ".ret"){
        auto func_type = std::dynamic_pointer_cast<FuncTypeExpr>(curr_func_type);
        TypeExprPtr return_type = func_type->get_return_type();
        if(dest.has_value()){
            error(stmt->get_token(), "Return instruction should not have a destination", "Function return type: " + return_type->to_string());
        }
        if(params.size() == 0){
            if(return_type->get_kind() != TypeExprKind::VoidTypeExpr){
                error(stmt->get_token(), "Return statement missing return value", "Function return type: " + return_type->to_string());
            }
        }
        else if(params.size() == 1){
            auto ret_param = params[0];
            if(!TypeUtils::is_type_equal(ret_param->get_type(), return_type)){
                error(ret_param->get_token(), "Return statement type mismatch", "Function return type: " + return_type->to_string() + ", Return statement type: " + ret_param->get_type()->to_string());
            }
        }
        else{
            error(stmt->get_token(), "Return statement has more than one parameter", "Function return type: " + return_type->to_string());
        }
    }
    else if(inst_name == ".call"){//Used for both external and internal function calls.
        if(params.size() != 1){
            error(stmt->get_token(), "Call instruction expects exactly one parameter", "Number of parameters: " + std::to_string(params.size()));
        }
        auto call_param = params[0];
        if(call_param->get_type()->get_kind() != TypeExprKind::FuncTypeExpr){
            error(call_param->get_token(), "Call instruction expects a function type as the first parameter", "Call parameter type: " + call_param->get_type()->to_string());
        }
        auto func_type = std::dynamic_pointer_cast<FuncTypeExpr>(call_param->get_type());
        if(dest.has_value()){
            if(!TypeUtils::is_type_equal(dest.value().second, func_type->get_return_type())){
                error(dest.value().first, "Call instruction destination type mismatch", "Function return type: " + func_type->get_return_type()->to_string() + ", Destination type: " + dest.value().second->to_string());
            }
        }
        else{
            if(func_type->get_return_type()->get_kind() != TypeExprKind::VoidTypeExpr){
                error(stmt->get_token(), "Call instruction missing destination for non-void function", "Function return type: " + func_type->get_return_type()->to_string());
            }
        }
    }
    else if(inst_name == ".get_field"){
        if(params.size() != 1){
            error(stmt->get_token(), "Get field instruction expects exactly one parameter", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::StructTypeExpr){
            error(params[0]->get_token(), "Get field instruction expects a struct type as the first parameter", "Get field parameter type: " + params[0]->get_type()->to_string());
        }
        if(params[1]->get_type()->get_kind() != TypeExprKind::IntTypeExpr || params[1]->get_kind() != LiteralKind::NumLiteralExpr){
            error(params[1]->get_token(), "Get field instruction expects an constexpr int type as the second parameter", "Get field parameter type: " + params[1]->get_type()->to_string());
        }
        auto struct_type = std::dynamic_pointer_cast<StructTypeExpr>(params[0]->get_type());
        std::int64_t field_index = std::stoll(params[1]->get_token().value);
        if(field_index < 0 || field_index >= struct_type->get_fields().size()){
            error(params[1]->get_token(), "Get field instruction field index out of bounds", "Field index: " + std::to_string(field_index) + ", Struct field count: " + std::to_string(struct_type->get_fields().size()));
        }
        if(dest.has_value()){
            if(!TypeUtils::is_type_equal(dest.value().second, struct_type->get_fields()[field_index])){
                error(dest.value().first, "Get field instruction destination type mismatch", "Struct field type: " + struct_type->get_fields()[field_index]->to_string() + ", Destination type: " + dest.value().second->to_string());
            }
        }
        else{
            error(stmt->get_token(), "Get field instruction missing destination", "Struct field type: " + struct_type->get_fields()[field_index]->to_string());
        }
    }
    else if(inst_name == ".set_field"){
        if(params.size() != 3){
            error(stmt->get_token(), "Set field instruction expects exactly three parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(dest.has_value()){
            error(stmt->get_token(), "Set field instruction should not have a destination", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::StructTypeExpr){
            error(params[0]->get_token(), "Set field instruction expects a struct type as the first parameter", "Set field parameter type: " + params[0]->get_type()->to_string());
        }
        if(params[1]->get_type()->get_kind() != TypeExprKind::IntTypeExpr || params[1]->get_kind() != LiteralKind::NumLiteralExpr){
            error(params[1]->get_token(), "Set field instruction expects an constexpr int type as the second parameter", "Set field parameter type: " + params[1]->get_type()->to_string());
        }
        auto struct_type = std::dynamic_pointer_cast<StructTypeExpr>(params[0]->get_type());
        std::int64_t field_index = std::stoll(params[1]->get_token().value);
        if(field_index < 0 || field_index >= struct_type->get_fields().size()){
            error(params[1]->get_token(), "Set field instruction field index out of bounds", "Field index: " + std::to_string(field_index) + ", Struct field count: " + std::to_string(struct_type->get_fields().size()));
        }
        if(!TypeUtils::is_type_equal(params[2]->get_type(), struct_type->get_fields()[field_index])){
            error(params[2]->get_token(), "Set field instruction value type mismatch", "Struct field type: " + struct_type->get_fields()[field_index]->to_string() + ", Value type: " + params[2]->get_type()->to_string());
        }
    }
    else if(inst_name == ".get_elem"){
        if(params.size() != 2){
            error(stmt->get_token(), "Get element instruction expects exactly two parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::ArrayTypeExpr && params[0]->get_type()->get_kind() != TypeExprKind::PtrTypeExpr && params[0]->get_type()->get_kind() != TypeExprKind::StringTypeExpr){
            error(params[0]->get_token(), "Get element instruction expects an array/string/pointer type as the first parameter", "Get element parameter type: " + params[0]->get_type()->to_string());
        }
        if(params[1]->get_type()->get_kind() != TypeExprKind::IntTypeExpr){
            error(params[1]->get_token(), "Get element instruction expects an int type as the second parameter", "Get element parameter type: " + params[1]->get_type()->to_string());
        }
        if(dest.has_value()){
            if(params[0]->get_type()->get_kind() == TypeExprKind::ArrayTypeExpr){
                auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(params[0]->get_type());
                if(!TypeUtils::is_type_equal(dest.value().second, array_type->get_basetype())){
                    error(dest.value().first, "Get element instruction destination type mismatch", "Array base type: " + array_type->get_basetype()->to_string() + ", Destination type: " + dest.value().second->to_string());
                }
            }
            else if(params[0]->get_type()->get_kind() == TypeExprKind::StringTypeExpr){
                if(!TypeUtils::is_type_equal(dest.value().second, params[0]->get_type())){
                    error(dest.value().first, "Get element instruction destination type mismatch", "String base type: " + params[0]->get_type()->to_string() + ", Destination type: " + dest.value().second->to_string());
                }
            }
            //We dont worry about the element size of ptr. Regardless of size, we read the same number of bytes as the destination type. NOTE:The is_missing field is also read from the ptr for type == ptr. For array it depends if packed or not(Read for unpacked. And for packed the entire array is missing if one item is missing so we just check that). String is just a packed array
        }
        else{
            error(stmt->get_token(), "Get element instruction missing destination");
        }
    }
    else if(inst_name == ".set_elem"){
        if(params.size() != 3){
            error(stmt->get_token(), "Set element instruction expects exactly three parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::ArrayTypeExpr && params[0]->get_type()->get_kind() != TypeExprKind::PtrTypeExpr && params[0]->get_type()->get_kind() != TypeExprKind::StringTypeExpr){
            error(params[0]->get_token(), "Set element instruction expects an array/string/pointer type as the first parameter", "Set element parameter type: " + params[0]->get_type()->to_string());
        }
        if(params[1]->get_type()->get_kind() != TypeExprKind::IntTypeExpr){
            error(params[1]->get_token(), "Set element instruction expects an int type as the second parameter", "Set element parameter type: " + params[1]->get_type()->to_string());
        }
        if(params[0]->get_type()->get_kind() == TypeExprKind::ArrayTypeExpr){
            auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(params[0]->get_type());
            if(!TypeUtils::is_type_equal(params[2]->get_type(), array_type->get_basetype())){
                error(params[2]->get_token(), "Set element instruction value type mismatch", "Array base type: " + array_type->get_basetype()->to_string() + ", Value type: " + params[2]->get_type()->to_string());
            }
        }
        else if(params[0]->get_type()->get_kind() == TypeExprKind::StringTypeExpr){
            if(!TypeUtils::is_type_equal(params[2]->get_type(), params[0]->get_type())){
                error(params[2]->get_token(), "Set element instruction value type mismatch", "String base type: " + params[0]->get_type()->to_string() + ", Value type: " + params[2]->get_type()->to_string());
            }
        }
        //We dont worry about the element size of ptr. Regardless of size, we write the same number of bytes as the destination type.NOTE:The is_missing field is also copied to the ptr for type == ptr. For array it depends if packed or not(Copied for unpacked. And for packed the entire thing is missing if one item is missing). String is just a packed array
    }
    else if(inst_name == ".weighed_add"){
        if(params.size() < 3 || (params.size() - 1) % 2 != 0){
            error(stmt->get_token(), "Weighted add instruction expects atleast 3 parameters and an odd number of parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        auto type = params[0]->get_type();
        if(type->get_kind() != TypeExprKind::IntTypeExpr && type->get_kind() != TypeExprKind::DecimalTypeExpr){
            error(params[0]->get_token(), "Weighted add instruction expects the first parameter to be either int or float type", "First parameter type: " + type->to_string());
        }
        for(std::size_t i = 1; i < params.size(); i ++){
            if(!TypeUtils::is_type_equal(params[i]->get_type(), type)){
                error(params[i]->get_token(), "Weighted add instruction parameter type mismatch", "Expected type: " + type->to_string() + ", Parameter type: " + params[i]->get_type()->to_string());
            }
        }
        if(dest.has_value()){
            if(!TypeUtils::is_type_equal(dest.value().second, type)){
                error(dest.value().first, "Weighted add instruction destination type mismatch", "Expected type: " + type->to_string() + ", Destination type: " + dest.value().second->to_string());
            }
        }
        else{
            error(stmt->get_token(), "Weighted add instruction missing destination");
        }
    }
    else{
        //TODO:
    }
}
}
}
}