#include "ir/parser/parser.hpp"
#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <iostream>
#include "instruction_arg_type.hpp"

namespace Hazel {
namespace Snap{
namespace IR {
void Parser::typecheck_inst(InstructionStmtPtr stmt, TypeExprPtr curr_func_type, bool last_inst) const{
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
        //Copy but not deep copy. So for let's say an array, the dest will have a pointer to the same underlying array. SO the address got copied but not the underlying content. Same for struct and string. For integer/decimal regular copy as u would expect
        if(params.size() != 2){
            error(stmt->get_token(), "Get field instruction expects exactly two parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::StructTypeExpr){
            error(params[0]->get_token(), "Get field instruction expects a struct type as the first parameter", "Get field parameter type: " + params[0]->get_type()->to_string());
        }
        if(params[1]->get_type()->get_kind() != TypeExprKind::IntTypeExpr || params[1]->get_kind() != LiteralKind::NumLiteralExpr){
            error(params[1]->get_token(), "Get field instruction expects an constexpr int type as the second parameter", "Get field parameter type: " + params[1]->get_type()->to_string());
        }
        auto struct_type = std::dynamic_pointer_cast<StructTypeExpr>(params[0]->get_type());
        std::uint64_t field_index = std::stoull(params[1]->get_token().value);
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
        //For setting string/array/struct, we write the pointer to the underlying content but not deep copy. Since the memory is already allocated+idx gurrentee to be in bound, so we just write it without worrying about overflow i.e it doesnt need to return the struct because we dont realloc
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
        std::uint64_t field_index = std::stoull(params[1]->get_token().value);
        if(field_index < 0 || field_index >= struct_type->get_fields().size()){
            error(params[1]->get_token(), "Set field instruction field index out of bounds", "Field index: " + std::to_string(field_index) + ", Struct field count: " + std::to_string(struct_type->get_fields().size()));
        }
        if(!TypeUtils::is_type_equal(params[2]->get_type(), struct_type->get_fields()[field_index])){
            error(params[2]->get_token(), "Set field instruction value type mismatch", "Struct field type: " + struct_type->get_fields()[field_index]->to_string() + ", Value type: " + params[2]->get_type()->to_string());
        }
    }
    else if(inst_name == ".get_elem"){
        if(params.size() != 2){
            error(stmt->get_token(), "Get element instruction expects exactly 2 parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::ArrayTypeExpr && params[0]->get_type()->get_kind() != TypeExprKind::StringTypeExpr){
            error(params[0]->get_token(), "Get element instruction expects an array/string type as the first parameter", "Get element parameter type: " + params[0]->get_type()->to_string());
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
        }
        else{
            error(stmt->get_token(), "Get element instruction missing destination");
        }
    }
    else if(inst_name == ".set_elem"){
        //For setting string/struct/array, we write the pointer to the underlying content but not deep copy. Since the index here is runtime so we return the array/string after setting the element because we may return missing for that
        if(params.size() != 3){
            error(stmt->get_token(), "Set element instruction expects exactly three parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::ArrayTypeExpr && params[0]->get_type()->get_kind() != TypeExprKind::StringTypeExpr){
            error(params[0]->get_token(), "Set element instruction expects an array/string type as the first parameter", "Set element parameter type: " + params[0]->get_type()->to_string());
        }
        if(dest.has_value()){
            if(!TypeUtils::is_type_equal(dest.value().second, params[0]->get_type())){
                error(dest.value().first, "Set element instruction destination type mismatch", "Set element parameter type: " + params[0]->get_type()->to_string() + ", Destination type: " + dest.value().second->to_string());
            }
        }
        else{
            error(stmt->get_token(), "Set element instruction missing destination");
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
    }
    else if(inst_name == ".push_elm"){
        if(params.size() != 2){
            error(stmt->get_token(), "Push element instruction expects exactly two parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::ArrayTypeExpr){
            error(params[0]->get_token(), "Push element instruction expects an array type as the first parameter", "Push element parameter type: " + params[0]->get_type()->to_string());
        }
        if(dest.has_value()){
            if(!TypeUtils::is_type_equal(dest.value().second, params[0]->get_type())){
                error(dest.value().first, "Push element instruction destination type mismatch", "Push element parameter type: " + params[0]->get_type()->to_string() + ", Destination type: " + dest.value().second->to_string());
            }
        }
        else{
            error(stmt->get_token(), "Push element instruction missing destination");
        }
        auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(params[0]->get_type());
        if(!TypeUtils::is_type_equal(params[1]->get_type(), array_type->get_basetype())){
            error(params[1]->get_token(), "Push element instruction value type mismatch", "Array base type: " + array_type->get_basetype()->to_string() + ", Value type: " + params[1]->get_type()->to_string());
        }
    }
    else if(inst_name == ".push_elm_at"){
        if(params.size() != 3){
            error(stmt->get_token(), "Push element at instruction expects exactly three parameters", "Number of parameters: " + std::to_string(params.size()));
        }
        if(params[0]->get_type()->get_kind() != TypeExprKind::ArrayTypeExpr){
            error(params[0]->get_token(), "Push element at instruction expects an array type as the first parameter", "Push element at parameter type: " + params[0]->get_type()->to_string());
        }
        if(params[1]->get_type()->get_kind() != TypeExprKind::IntTypeExpr){
            error(params[1]->get_token(), "Push element at instruction expects an int type as the second parameter", "Push element at parameter type: " + params[1]->get_type()->to_string());
        }
        if(dest.has_value()){
            if(!TypeUtils::is_type_equal(dest.value().second, params[0]->get_type())){
                error(dest.value().first, "Push element at instruction destination type mismatch", "Push element at parameter type: " + params[0]->get_type()->to_string() + ", Destination type: " + dest.value().second->to_string());
            }
        }
        else{
            error(stmt->get_token(), "Push element at instruction missing destination");
        }
        auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(params[0]->get_type());
        if(!TypeUtils::is_type_equal(params[2]->get_type(), array_type->get_basetype())){
            error(params[2]->get_token(), "Push element at instruction value type mismatch", "Array base type: " + array_type->get_basetype()->to_string() + ", Value type: " + params[2]->get_type()->to_string());
        }
    }
    else if(inst_name == ".get_elem_ptr"){
        // GEP(val, idx0, idx1, ...): walks a chain of struct-field/array-element/string-element
        // indices starting from `val`, returning the address of the final location.
        // No indices -> address of val itself.
        // Struct step: index must be constexpr int, bounds-checked at compile time.
        // Array step: index may be any runtime int, unchecked here (checked at deref via
        //   the value's own length/capacity - GEP itself never dereferences).
        // String step: index may be any runtime int, unchecked here. A string element is
        //   itself typed as StringTypeExpr (single "char" string) and is terminal - no
        //   further index may follow a string step.
        if(params.size() < 1){
            error(stmt->get_token(), "Get element pointer instruction expects at least one parameter", "Number of parameters: " + std::to_string(params.size()));
        }

        TypeExprPtr curr_type = params[0]->get_type();
        for(std::size_t i = 1; i < params.size(); i ++){
            auto idx_param = params[i];
            auto curr_kind = curr_type->get_kind();

            if(curr_kind == TypeExprKind::StructTypeExpr){
                if(idx_param->get_type()->get_kind() != TypeExprKind::IntTypeExpr){
                    error(idx_param->get_token(), "Get element pointer instruction expects an int type index for struct step", "Index parameter type: " + idx_param->get_type()->to_string());
                }
                if(idx_param->get_kind() != LiteralKind::NumLiteralExpr){
                    error(idx_param->get_token(), "Get element pointer instruction expects a constexpr int index for struct step", "Index parameter type: " + idx_param->get_type()->to_string());
                }
                auto struct_type = std::dynamic_pointer_cast<StructTypeExpr>(curr_type);
                std::uint64_t field_index = std::stoull(idx_param->get_token().value);
                if(field_index < 0 || field_index >= struct_type->get_fields().size()){
                    error(idx_param->get_token(), "Get element pointer instruction field index out of bounds", "Field index: " + std::to_string(field_index) + ", Struct field count: " + std::to_string(struct_type->get_fields().size()));
                }
                curr_type = struct_type->get_fields()[field_index];
            }
            else if(curr_kind == TypeExprKind::ArrayTypeExpr){
                if(idx_param->get_type()->get_kind() != TypeExprKind::IntTypeExpr){
                    error(idx_param->get_token(), "Get element pointer instruction expects an int type index for array step", "Index parameter type: " + idx_param->get_type()->to_string());
                }
                auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(curr_type);
                curr_type = array_type->get_basetype();
            }
            else if(curr_kind == TypeExprKind::StringTypeExpr){
                if(idx_param->get_type()->get_kind() != TypeExprKind::IntTypeExpr){
                    error(idx_param->get_token(), "Get element pointer instruction expects an int type index for string step", "Index parameter type: " + idx_param->get_type()->to_string());
                }
                if(i != params.size() - 1){
                    error(params[i + 1]->get_token(), "Get element pointer instruction cannot index further into a string element", "String step must be the last index");
                }
                curr_type = curr_type; // string element is itself StringTypeExpr, terminal
            }
            else{
                error(idx_param->get_token(), "Get element pointer instruction cannot index into this type", "Type at this step: " + curr_type->to_string());
            }
        }

        if(dest.has_value()){
            if(dest.value().second->get_kind() != TypeExprKind::PtrTypeExpr){
                error(dest.value().first, "Get element pointer instruction destination must be ptr type", "Destination type: " + dest.value().second->to_string());
            }
        }
        else{
            error(stmt->get_token(), "Get element pointer instruction missing destination", "Destination type: ptr");
        }
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
        auto it = instruction_arg_type_map.find(inst_name);
        if(it == instruction_arg_type_map.end()){
            error(stmt->get_token(), "Unknown instruction", "Instruction name: " + inst_name);
        }
        auto valid_arg_types = it->second;
        bool valid = false;
        for(auto& valid_types : valid_arg_types){
            if(valid_types.size() - 1 != params.size()){
                continue;
            }
            TypeExprPtr any_type = nullptr; 
            if(valid_types.back() == TypeExprKind::VoidTypeExpr && !dest.has_value()){}
            else if(valid_types.back() != TypeExprKind::VoidTypeExpr && dest.has_value()){
                if(dest.value().second->get_kind() != valid_types.back()){
                    if(valid_types.back() == TypeExprKind::AnyTypeExpr){
                        any_type = dest.value().second;
                    }
                    else{
                        continue;
                    }
                }
            }
            else{
                continue;
            }
            bool valid_params = true;
            for(std::size_t i = 0; i < params.size(); i ++){
                if(params[i]->get_type()->get_kind() != valid_types[i]){
                    if(valid_types[i] == TypeExprKind::AnyTypeExpr){
                        if(any_type == nullptr){
                            any_type = params[i]->get_type();
                        }
                        else{
                            if(!TypeUtils::is_type_equal(any_type, params[i]->get_type(), false)){
                                valid_params = false;
                                break;
                            }
                        }
                    }
                    else{
                        valid_params = false;
                        break;
                    }
                }
            }
            if(valid_params){
                valid = true;
                break;
            }
        }
        if(!valid){
            error(stmt->get_token(), "Instruction parameter/destination type mismatch", "Instruction name: " + inst_name);
        }
    }
    auto is_terminator = terminator_inst.find(inst_name) != terminator_inst.end();
    if(is_terminator && !last_inst){
        error(stmt->get_token(), "Terminator instruction should be the last instruction in a label", "Instruction name: " + inst_name);
    }
    else if(!is_terminator && last_inst){
        error(stmt->get_token(), "Non-terminator instruction should not be the last instruction in a label", "Instruction name: " + inst_name);
    }
}
}
}
}