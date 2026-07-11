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


InstructionStmt::InstructionStmt(Token tok, Token instruction, std::optional<std::pair<Token, TypeExprPtr>> name, std::vector<LiteralExprPtr> params, DebugInfoPtr debug_info){
    this->tok = tok;
    this->instruction = instruction;
    this->name = name;
    this->params = params;
    this->debug_info = debug_info;
}
Token InstructionStmt::get_instruction() const{
    return this->instruction;
}
std::optional<std::pair<Token, TypeExprPtr>> InstructionStmt::get_name() const{
    return this->name;
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
        res = "let " + this->name.value().second->to_string() + ":" + this->name.value().first.value + " = ";
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
    res += "}";
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


Function::Function(Token tok, Token name, std::vector<std::pair<Token, TypeExprPtr>> params, 
                   std::vector<LabelPtr> body, DebugInfoPtr debug_info, bool calculate_maps){
    this->tok = tok;
    this->name = name;
    this->params = params;
    this->body = body;
    this->debug_info = debug_info;
    if(calculate_maps){
        for(size_t i=0;i<this->params.size();i++){
            this->parameter_map[this->params[i].first.value] = i;
        }
        for(size_t i=0;i<this->body.size();i++){
            this->label_map[this->body[i]->get_name().value] = i;
            for(const auto& stmt: this->body[i]->get_statements()){
                auto stmt_name = stmt->get_name();
                if(stmt_name.has_value()){
                    this->local_var_map[stmt_name.value().first.value] = i;
                }
            }
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
    res += ")";
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
std::optional<std::pair<Token, LabelPtr>> Function::get_label_of_local_var(std::string name) const{
    auto it = this->local_var_map.find(name);
    if(it == this->local_var_map.end()){
        return std::nullopt;
    }
    auto label = this->body[it->second];
    auto local_var = label->get_local_var(name);
    return std::make_pair(local_var.value().first, label);
}
std::optional<Utils::triplet<Token, TypeExprPtr, bool>> Function::get_local_var_or_param(std::string name) const{
    auto param_it = this->parameter_map.find(name);
    if(param_it != this->parameter_map.end()){
        auto param = this->params[param_it->second];
        return Utils::triplet<Token, TypeExprPtr, bool>(param.first, param.second, true);
    }
    auto local_it = this->local_var_map.find(name);
    if(local_it != this->local_var_map.end()){
        auto label = this->body[local_it->second];
        auto local_var = label->get_local_var(name);
        return Utils::triplet<Token, TypeExprPtr, bool>(local_var.value().first, local_var.value().second, false);
    }
    return std::nullopt;
}


Program::Program(std::vector<FunctionPtr> items, bool calculate_map){
    this->items = items;
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
}
}
}