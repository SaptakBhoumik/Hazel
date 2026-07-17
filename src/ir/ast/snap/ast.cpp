#include "ir/ast/ast.hpp"

namespace Hazel {
namespace Snap {
namespace IR {
DebugInfo::DebugInfo(Token tok, Token file_name, Token line, Token column){
    this->tok = tok;
    this->file_name = file_name;
    this->line = line;
    this->column = column;
}
Token DebugInfo::get_file_name() const{
    return this->file_name;
}
Token DebugInfo::get_line() const{
    return this->line;
}
Token DebugInfo::get_column() const{
    return this->column;
}
Token DebugInfo::get_token() const{
    return this->tok;
}
std::string DebugInfo::to_string() const{
    return "\""+this->file_name.value+"\":"+this->line.value+":"+this->column.value;
}


InstructionStmt::InstructionStmt(Token tok, Token instruction, std::optional<std::pair<Token, TypeExprPtr>> name, std::optional<std::string> original_name, std::vector<LiteralExprPtr> params, DebugInfoPtr debug_info){
    this->tok = tok;
    this->instruction = instruction;
    this->name = name;
    this->original_name = original_name;
    this->params = params;
    this->debug_info = debug_info;
}
Token InstructionStmt::get_instruction() const{
    return this->instruction;
}
std::optional<std::pair<Token, TypeExprPtr>> InstructionStmt::get_name() const{
    return this->name;
}
std::optional<std::string> InstructionStmt::get_original_name() const{
    return this->original_name;
}
std::vector<LiteralExprPtr> InstructionStmt::get_params() const{
    return this->params;
}
DebugInfoPtr InstructionStmt::get_debug_info() const{
    return this->debug_info;
}
Token InstructionStmt::get_token() const{
    return this->tok;
}
std::string InstructionStmt::to_string() const{
    std::string res;
    if(this->name.has_value()){
        res = "let " + this->name.value().second->to_string() + ":" + this->name.value().first.value;
        if(this->original_name.has_value()){
            res += ":\"" + this->original_name.value() + "\"";
        }
        res += " = ";
    }
    res += this->instruction.value + "(";
    for(size_t i=0;i<this->params.size();i++){
        res += this->params[i]->to_string();
        if(i!=this->params.size()-1){
            res += ", ";
        }
    }
    res += ")";
    if(this->debug_info != nullptr){
        res += " ! " + this->debug_info->to_string();
    }
    return res;
}


Label::Label(Token tok, Token name, std::vector<InstructionStmtPtr> statements, std::vector<std::pair<Token, TypeExprPtr>> params, DebugInfoPtr debug_info, bool calculate_maps){
    this->tok = tok;
    this->name = name;
    this->statements = statements;
    this->params = params;
    this->debug_info = debug_info;
    if(calculate_maps){
        for(size_t i=0;i<this->params.size();i++){
            this->parameter_map[this->params[i].first.value] = i;
        }
        for(size_t i=0;i<this->statements.size();i++){
            auto stmt_name = this->statements[i]->get_name();
            if(stmt_name.has_value()){
                this->local_var_map[stmt_name.value().first.value] = i;
            }
        }
    }
}
Token Label::get_name() const{
    return this->name;
}
Token Label::get_token() const{
    return this->tok;
}
std::vector<InstructionStmtPtr> Label::get_statements() const{
    return this->statements;
}
std::vector<std::pair<Token, TypeExprPtr>> Label::get_params() const{
    return this->params;
}
DebugInfoPtr Label::get_debug_info() const{
    return this->debug_info;
}
std::string Label::to_string() const{
    std::string res = "\tlabel " + this->name.value + "(";
    for(size_t i=0;i<this->params.size();i++){
        res += "let " + this->params[i].second->to_string() + ":" + this->params[i].first.value;
        if(i!=this->params.size()-1){
            res += ", ";
        }
    }
    res += ")";
    if(this->debug_info != nullptr){
        res += " ! " + this->debug_info->to_string();
    }
    res += " {\n";
    for(const auto& stmt: this->statements){
        res += "\t\t" + stmt->to_string() + ";\n";
    }
    res += "\t}";
    return res;
}
std::optional<std::pair<Token, TypeExprPtr>> Label::get_parameter(std::string name) const{
    auto it = this->parameter_map.find(name);
    if(it == this->parameter_map.end()){
        return std::nullopt;
    }
    return this->params[it->second];
}
std::optional<std::pair<Token, InstructionStmtPtr>> Label::get_local_var_inst(std::string name) const{
    auto it = this->local_var_map.find(name);
    if(it == this->local_var_map.end()){
        return std::nullopt;
    }
    auto stmt = this->statements[it->second];
    return std::make_pair(stmt->get_name().value().first, stmt);
}
std::optional<std::pair<Token, TypeExprPtr>> Label::get_local_var(std::string name) const{
    auto it = this->local_var_map.find(name);
    if(it == this->local_var_map.end()){
        return std::nullopt;
    }
    return this->statements[it->second]->get_name();
}
std::optional<Utils::triplet<Token, TypeExprPtr, bool>> Label::get_local_var_or_param(std::string name) const{
    auto param_it = this->parameter_map.find(name);
    if(param_it != this->parameter_map.end()){
        auto param = this->params[param_it->second];
        return Utils::triplet<Token, TypeExprPtr, bool>(param.first, param.second, true);
    }
    auto local_it = this->local_var_map.find(name);
    if(local_it != this->local_var_map.end()){
        auto local_var = this->statements[local_it->second]->get_name();
        return Utils::triplet<Token, TypeExprPtr, bool>(local_var.value().first, local_var.value().second, false);
    }
    return std::nullopt;
}


Function::Function(Token tok, Token name, std::vector<std::pair<Token, TypeExprPtr>> params, TypeExprPtr return_type,
                   std::vector<LabelPtr> body, DebugInfoPtr debug_info, bool calculate_maps){
    this->tok = tok;
    this->name = name;
    this->params = params;
    this->return_type = return_type;
    this->body = body;
    this->debug_info = debug_info;
    if(calculate_maps){
        for(size_t i=0;i<this->params.size();i++){
            this->parameter_map[this->params[i].first.value] = i;
        }
        for(size_t i=0;i<this->body.size();i++){
            this->label_map[this->body[i]->get_name().value] = i;
            // for(const auto& stmt: this->body[i]->get_statements()){
            //     auto stmt_name = stmt->get_name();
            //     if(stmt_name.has_value()){
            //         this->local_var_map[stmt_name.value().first.value] = i;
            //     }
            // }
        }
    }
}
Token Function::get_name() const{
    return this->name;
}
Token Function::get_token() const{
    return this->tok;
}
std::vector<std::pair<Token, TypeExprPtr>> Function::get_params() const{
    return this->params;
}
TypeExprPtr Function::get_return_type() const{
    return this->return_type;
}
bool Function::is_signature_only() const{
    return this->body.empty();
}
std::vector<LabelPtr> Function::get_body() const{
    return this->body;
}
DebugInfoPtr Function::get_debug_info() const{
    return this->debug_info;
}
std::string Function::to_string() const{
    std::string res = "fn " + this->name.value + "(";
    for(size_t i=0;i<this->params.size();i++){
        res += "let " + this->params[i].second->to_string() + ":" + this->params[i].first.value;
        if(i!=this->params.size()-1){
            res += ", ";
        }
    }
    res += ")->" + this->return_type->to_string();
    if(this->debug_info != nullptr){
        res += " ! " + this->debug_info->to_string();
    }
    if(!this->body.empty()){
        res += " {\n";
        for(const auto& label: this->body){
            res += label->to_string() + "\n";
        }
        res += "}";
    }
    else{
        res += ";";
    }
    return res;
}
std::optional<std::pair<Token, TypeExprPtr>> Function::get_parameter(std::string name) const{
    auto it = this->parameter_map.find(name);
    if(it == this->parameter_map.end()){
        return std::nullopt;
    }
    return this->params[it->second];
}
std::optional<std::pair<Token, LabelPtr>> Function::get_label(std::string name) const{
    auto it = this->label_map.find(name);
    if(it == this->label_map.end()){
        return std::nullopt;
    }
    auto label = this->body[it->second];
    return std::make_pair(label->get_name(), label);
}
bool Function::is_param(std::string name) const{
    return this->parameter_map.find(name) != this->parameter_map.end();
}
std::optional<std::pair<Token,TypeExprPtr>> Function::resolve_name(std::string var_name, std::string label_name) const{
    // First check if the variable is a parameter
    if(is_param(var_name)){
        return this->get_parameter(var_name);
    }
    // Then check if the variable is in the specified label
    auto label_it = this->label_map.find(label_name);
    if(label_it == this->label_map.end()){
        return std::nullopt;
    }
    auto label = this->body[label_it->second];
    return label->get_local_var(var_name);
}
// std::optional<std::pair<Token, LabelPtr>> Function::get_label_of_local_var(std::string name) const{
//     auto it = this->local_var_map.find(name);
//     if(it == this->local_var_map.end()){
//         return std::nullopt;
//     }
//     auto label = this->body[it->second];
//     auto local_var = label->get_local_var(name);
//     return std::make_pair(local_var.value().first, label);
// }
// std::optional<Utils::triplet<Token, TypeExprPtr, bool>> Function::get_local_var_or_param(std::string name) const{
//     auto param_it = this->parameter_map.find(name);
//     if(param_it != this->parameter_map.end()){
//         auto param = this->params[param_it->second];
//         return Utils::triplet<Token, TypeExprPtr, bool>(param.first, param.second, true);
//     }
//     auto local_it = this->local_var_map.find(name);
//     if(local_it != this->local_var_map.end()){
//         auto label = this->body[local_it->second];
//         auto local_var = label->get_local_var(name);
//         return Utils::triplet<Token, TypeExprPtr, bool>(local_var.value().first, local_var.value().second, false);
//     }
//     return std::nullopt;
// }


Program::Program(std::vector<FunctionPtr> items, std::optional<Utils::Graph> call_graph, bool calculate_map){
    this->items = items;
    this->call_graph = call_graph;
    if(calculate_map){
        for(size_t i=0;i<this->items.size();i++){
            this->function_map[this->items[i]->get_name().value] = i;
        }
    }
}
std::vector<FunctionPtr> Program::get_items() const{
    return this->items;
}
std::optional<std::pair<Token, FunctionPtr>> Program::get_function(std::string name) const{
    auto it = this->function_map.find(name);
    if(it == this->function_map.end()){
        return std::nullopt;
    }
    auto func = this->items[it->second];
    return std::make_pair(func->get_name(), func);
}
std::string Program::to_string() const{
    std::string res;
    for(const auto& item: this->items){
        res += item->to_string() + "\n";
    }
    return res;
}

namespace TypeUtils{
bool is_type_equal(TypeExprPtr type1, TypeExprPtr type2, bool check_packing_and_underlying_size){
    if(type1->get_kind() != type2->get_kind()){
        return false;
    }
    switch(type1->get_kind()){
        case TypeExprKind::NamedTypeExpr:{
            auto named_type1 = std::dynamic_pointer_cast<NamedTypeExpr>(type1);
            auto named_type2 = std::dynamic_pointer_cast<NamedTypeExpr>(type2);
            return named_type1->get_name().value == named_type2->get_name().value;
        }
        case TypeExprKind::IntTypeExpr:
        case TypeExprKind::DecimalTypeExpr:
        case TypeExprKind::StringTypeExpr:
        case TypeExprKind::VoidTypeExpr:{
            return true;
        }
        case TypeExprKind::PtrTypeExpr:{
            auto ptr_type1 = std::dynamic_pointer_cast<PtrTypeExpr>(type1);
            auto ptr_type2 = std::dynamic_pointer_cast<PtrTypeExpr>(type2);
            if(!check_packing_and_underlying_size){
                return true;
            }
            return ptr_type1->get_element_size() == ptr_type2->get_element_size();
        }
        case TypeExprKind::ArrayTypeExpr:{
            auto array_type1 = std::dynamic_pointer_cast<ArrayTypeExpr>(type1);
            auto array_type2 = std::dynamic_pointer_cast<ArrayTypeExpr>(type2);
            if(!check_packing_and_underlying_size){
                return is_type_equal(array_type1->get_basetype(), array_type2->get_basetype(), check_packing_and_underlying_size);
            }
            return is_type_equal(array_type1->get_basetype(), array_type2->get_basetype(), check_packing_and_underlying_size) && array_type1->is_packed() == array_type2->is_packed();
        }
        case TypeExprKind::StructTypeExpr:{
            auto struct_type1 = std::dynamic_pointer_cast<StructTypeExpr>(type1);
            auto struct_type2 = std::dynamic_pointer_cast<StructTypeExpr>(type2);
            auto fields1 = struct_type1->get_fields();
            auto fields2 = struct_type2->get_fields();
            if(fields1.size() != fields2.size()){
                return false;
            }
            for(size_t i=0;i<fields1.size();i++){
                if(!is_type_equal(fields1[i], fields2[i], check_packing_and_underlying_size)){
                    return false;
                }
            }
            if(!check_packing_and_underlying_size){
                return true;
            }
            return struct_type1->is_packed() == struct_type2->is_packed();
        }
        case TypeExprKind::FuncTypeExpr:{
            auto func_type1 = std::dynamic_pointer_cast<FuncTypeExpr>(type1);
            auto func_type2 = std::dynamic_pointer_cast<FuncTypeExpr>(type2);
            auto params1 = func_type1->get_param_types();
            auto params2 = func_type2->get_param_types();
            if(params1.size() != params2.size()){
                return false;
            }
            for(size_t i=0;i<params1.size();i++){
                if(!is_type_equal(params1[i], params2[i], check_packing_and_underlying_size)){
                    return false;
                }
            }
            return is_type_equal(func_type1->get_return_type(), func_type2->get_return_type(), check_packing_and_underlying_size);
        }
        case TypeExprKind::LabelTypeExpr:{
            auto label_type1 = std::dynamic_pointer_cast<LabelTypeExpr>(type1);
            auto label_type2 = std::dynamic_pointer_cast<LabelTypeExpr>(type2);
            auto params1 = label_type1->get_param_types();
            auto params2 = label_type2->get_param_types();
            if(params1.size() != params2.size()){
                return false;
            }
            for(size_t i=0;i<params1.size();i++){
                if(!is_type_equal(params1[i], params2[i], check_packing_and_underlying_size)){
                    return false;
                }
            }
            return true;
        }
        case TypeExprKind::AnyTypeExpr:{
            return false;//The callee should check
        }
    }
    return false;
}
}
}
}
}