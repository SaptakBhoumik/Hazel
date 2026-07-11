#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <memory>

namespace Hazel {
namespace Snap {
namespace IR {
LiteralExpr::LiteralExpr(Token tok, LiteralKind kind, TypeExprPtr type){
    this->tok = tok;
    this->kind = kind;
    this->type = type;
}
void LiteralExpr::set_type(TypeExprPtr type){
    this->type = type;
}
void LiteralExpr::set_arg_type(){}
TypeExprPtr LiteralExpr::get_type() const{
    return this->type;
}
LiteralKind LiteralExpr::get_kind() const{
    return this->kind;
}
Token LiteralExpr::get_token() const{
    return this->tok;
}


ZeroInitLiteralExpr::ZeroInitLiteralExpr(Token tok, TypeExprPtr type)
    : LiteralExpr(tok, LiteralKind::ZeroInitLiteralExpr, type){
}
std::string ZeroInitLiteralExpr::to_string() const{
    return (this->type != nullptr ? this->type->to_string() + ": " : "") + "zeroinit";
}


PoisonLiteralExpr::PoisonLiteralExpr(Token tok, TypeExprPtr type)
    : LiteralExpr(tok, LiteralKind::PoisonLiteralExpr, type){
}
std::string PoisonLiteralExpr::to_string() const{
    return (this->type != nullptr ? this->type->to_string() + ": " : "") + "poison";
}


NamedLiteralExpr::NamedLiteralExpr(Token name, TypeExprPtr type)
    : LiteralExpr(name, LiteralKind::NamedLiteralExpr, type){
}
Token NamedLiteralExpr::get_name() const{
    return this->tok;
}
std::string NamedLiteralExpr::to_string() const{
    return (this->type != nullptr ? this->type->to_string() + ": " : "") + this->tok.value;
}


NumLiteralExpr::NumLiteralExpr(Token value, TypeExprPtr type)
    : LiteralExpr(value, LiteralKind::NumLiteralExpr, type){
}
Token NumLiteralExpr::get_value() const{
    return this->tok;
}
std::string NumLiteralExpr::to_string() const{
    return (this->type != nullptr ? this->type->to_string() + ": " : "") + this->tok.value;
}


StringLiteralExpr::StringLiteralExpr(Token value, TypeExprPtr type)
    : LiteralExpr(value, LiteralKind::StringLiteralExpr, type){
}
Token StringLiteralExpr::get_value() const{
    return this->tok;
}
std::string StringLiteralExpr::to_string() const{
    return (this->type != nullptr ? this->type->to_string() + ": " : "") + (this->tok.type == TokenType::raw_string ? "r\"":"\"") + this->tok.value + "\"";
}


ArrayLiteralExpr::ArrayLiteralExpr(Token tok, std::vector<LiteralExprPtr> elements, TypeExprPtr type)
    : LiteralExpr(tok, LiteralKind::ArrayLiteralExpr, type){
    this->elements = elements;
}
void ArrayLiteralExpr::set_arg_type(){
    auto array_type = std::dynamic_pointer_cast<ArrayTypeExpr>(this->type);
    for(auto& elem : this->elements){
        elem->set_type(array_type->get_basetype());
    }
}
std::vector<LiteralExprPtr> ArrayLiteralExpr::get_elements() const{
    return this->elements;
}
std::string ArrayLiteralExpr::to_string() const{
    std::string res = (this->type != nullptr ? this->type->to_string() + ": " : "") + "[";
    for(size_t i=0;i<this->elements.size();i++){
        res += this->elements[i]->to_string();
        if(i!=this->elements.size()-1){
            res += ", ";
        }
    }
    return res+"]";
}


StructLiteralExpr::StructLiteralExpr(Token tok, std::vector<LiteralExprPtr> fields, bool packed, TypeExprPtr type)
    : LiteralExpr(tok, LiteralKind::StructLiteralExpr, type){
    this->tok = tok;
    this->fields = fields;
    this->packed = packed;
}
void StructLiteralExpr::set_arg_type(){
    auto struct_fields = std::dynamic_pointer_cast<StructTypeExpr>(this->type)->get_fields();
    for(size_t i=0;i<this->fields.size();i++){
        this->fields[i]->set_type(struct_fields[i]);
    }
}
std::vector<LiteralExprPtr> StructLiteralExpr::get_fields() const{
    return this->fields;
}
bool StructLiteralExpr::is_packed() const{
    return this->packed;
}
std::string StructLiteralExpr::to_string() const{
    std::string res = (this->type != nullptr ? this->type->to_string() + ": " : "") + (this->packed ? "<{" : "{");
    for(size_t i=0;i<this->fields.size();i++){
        res += this->fields[i]->to_string();
        if(i!=this->fields.size()-1){
            res += ", ";
        }
    }
    return res+(this->packed ? "}>" : "}");
}


LabelLiteralExpr::LabelLiteralExpr(Token tok, std::vector<LiteralExprPtr> args, TypeExprPtr type)
    : LiteralExpr(tok, LiteralKind::LabelLiteralExpr, type){
    this->args = args;
}
Token LabelLiteralExpr::get_name() const{
    return this->tok;
}
void LabelLiteralExpr::set_arg_type(){
    auto label_param_type = std::dynamic_pointer_cast<LabelTypeExpr>(this->type)->get_param_types();
    for(size_t i=0;i<this->args.size();i++){
        this->args[i]->set_type(label_param_type[i]);
    }
}
std::vector<LiteralExprPtr> LabelLiteralExpr::get_args() const{
    return this->args;
}
std::string LabelLiteralExpr::to_string() const{
    std::string res = (this->type != nullptr ? this->type->to_string() + ": " : "") + this->tok.value + "(";
    for(size_t i=0;i<this->args.size();i++){
        res += this->args[i]->to_string();
        if(i!=this->args.size()-1){
            res += ", ";
        }
    }
    return res+")";
}


FuncLiteralExpr::FuncLiteralExpr(Token tok, std::vector<LiteralExprPtr> args, TypeExprPtr type)
    : LiteralExpr(tok, LiteralKind::FuncLiteralExpr, type){
    this->args = args;
}
Token FuncLiteralExpr::get_name() const{
    return this->tok;
}
void FuncLiteralExpr::set_arg_type(){
    auto func_param_type = std::dynamic_pointer_cast<FuncTypeExpr>(this->type)->get_param_types();
    for(size_t i=0;i<this->args.size();i++){
        this->args[i]->set_type(func_param_type[i]);
    }
}
std::vector<LiteralExprPtr> FuncLiteralExpr::get_args() const{
    return this->args;
}
std::string FuncLiteralExpr::to_string() const{
    std::string res = (this->type != nullptr ? this->type->to_string() + ": " : "") + this->tok.value + "(";
    for(size_t i=0;i<this->args.size();i++){
        res += this->args[i]->to_string();
        if(i!=this->args.size()-1){
            res += ", ";
        }
    }
    return res+")";
}
}
}
}