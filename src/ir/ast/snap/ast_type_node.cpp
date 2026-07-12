#include "ir/ast/ast.hpp"
#include <cctype>
#include <cstddef>

namespace Hazel {
namespace Snap {
namespace IR {
TypeExpr::TypeExpr(Token tok, TypeExprKind kind){
    this->tok = tok;
    this->kind = kind;
}
void TypeExpr::set_token(Token tok){
    this->tok = tok;
}
TypeExprKind TypeExpr::get_kind() const{
    return this->kind;
}
Token TypeExpr::get_token() const{
    return this->tok;
}


NamedTypeExpr::NamedTypeExpr(Token tok)
    : TypeExpr(tok, TypeExprKind::NamedTypeExpr){
    this->name = tok;
}
Token NamedTypeExpr::get_name() const{
    return this->name;
}
std::size_t NamedTypeExpr::get_size() const{
    return 0;
}
std::string NamedTypeExpr::to_string() const{
    return this->tok.value;
}


IntTypeExpr::IntTypeExpr(Token tok)
    : TypeExpr(tok, TypeExprKind::IntTypeExpr){
}
std::size_t IntTypeExpr::get_size() const{
    return 8+1;//8 bytes for value, 1 byte for is missing field
}
std::string IntTypeExpr::to_string() const{
    return "integer";
}


DecimalTypeExpr::DecimalTypeExpr(Token tok)
    : TypeExpr(tok, TypeExprKind::DecimalTypeExpr){
}
std::size_t DecimalTypeExpr::get_size() const{
    return 8+1;//8 bytes for value, 1 byte for is missing field
}
std::string DecimalTypeExpr::to_string() const{
    return "decimal";
}


StringTypeExpr::StringTypeExpr(Token tok)
    : TypeExpr(tok, TypeExprKind::StringTypeExpr){
}
std::size_t StringTypeExpr::get_size() const{
    return 8+8+8+2+1;//8 bytes for pointer, 8 bytes for length, 8 bytes for capacity, 2 bytes for element size, 1 byte for is missing field
}
std::string StringTypeExpr::to_string() const{
    return "string";
}


VoidTypeExpr::VoidTypeExpr()
    : TypeExpr(Token(), TypeExprKind::VoidTypeExpr){
}
std::size_t VoidTypeExpr::get_size() const{
    return 0;
}
std::string VoidTypeExpr::to_string() const{
    return "void";
}


PtrTypeExpr::PtrTypeExpr(Token tok, std::uint16_t element_size)
    : TypeExpr(tok, TypeExprKind::PtrTypeExpr){
    this->element_size = element_size;
}

std::uint16_t PtrTypeExpr::get_element_size() const{
    return this->element_size;
}
std::size_t PtrTypeExpr::get_size() const{
    return 8+8+8+2+1;//8 bytes for pointer, 8 bytes for length, 8 bytes for capacity, 2 bytes for element size, 1 byte for is missing field
}
std::string PtrTypeExpr::to_string() const{
    return "ptr<" + std::to_string(this->element_size) + ">";
}


ArrayTypeExpr::ArrayTypeExpr(Token tok, TypeExprPtr base_type)
    : TypeExpr(tok, TypeExprKind::ArrayTypeExpr){
    this->tok = tok;
    this->base_type = base_type;
}
TypeExprPtr ArrayTypeExpr::get_basetype() const{
    return this->base_type;
}
std::size_t ArrayTypeExpr::get_size() const{
    return 8+8+8+2+1;//8 bytes for pointer, 8 bytes for length, 8 bytes for capacity, 2 bytes for element size, 1 byte for is missing field
}
std::string ArrayTypeExpr::to_string() const{
    return "[" + this->base_type->to_string() + "]";
}


StructTypeExpr::StructTypeExpr(Token tok, std::vector<TypeExprPtr> fields, bool packed)
    : TypeExpr(tok, TypeExprKind::StructTypeExpr){
    this->tok = tok;
    this->fields = fields;
    this->packed = packed;
}
std::vector<TypeExprPtr> StructTypeExpr::get_fields() const{
    return this->fields;
}
bool StructTypeExpr::is_packed() const{
    return this->packed;
}
std::size_t StructTypeExpr::get_size() const{
    if(this->packed){
        return 8+8+8+2+1;//8 bytes for pointer, 8 bytes for length, 8 bytes for capacity, 2 bytes for element size, 1 byte for is missing field
    }
    std::size_t total = 0;
    for(const auto& field : this->fields){
        total += field->get_size();
    }
    return total;
}
std::string StructTypeExpr::to_string() const{
    std::string res = this->packed ? "<{" : "{";
    for(size_t i=0;i<this->fields.size();i++){
        res += this->fields[i]->to_string();
        if(i!=this->fields.size()-1){
            res += ", ";
        }
    }
    res += this->packed ? "}>" : "}";
    return res;
}


LabelTypeExpr::LabelTypeExpr(Token tok, std::vector<TypeExprPtr> param_types)
    : TypeExpr(tok, TypeExprKind::LabelTypeExpr){
    this->param_types = param_types;
}
std::vector<TypeExprPtr> LabelTypeExpr::get_param_types() const{
    return this->param_types;
}
std::size_t LabelTypeExpr::get_size() const{
    return 0;
}
std::string LabelTypeExpr::to_string() const{
    std::string res = "label(";
    for(size_t i=0;i<this->param_types.size();i++){
        res += this->param_types[i]->to_string();
        if(i!=this->param_types.size()-1){
            res += ", ";
        }
    }
    res += ")";
    return res;
}


FuncTypeExpr::FuncTypeExpr(Token tok, std::vector<TypeExprPtr> param_types, TypeExprPtr return_type)
    : TypeExpr(tok, TypeExprKind::FuncTypeExpr){
    this->tok = tok;
    this->param_types = param_types;
    this->return_type = return_type;
}
std::vector<TypeExprPtr> FuncTypeExpr::get_param_types() const{
    return this->param_types;
}
TypeExprPtr FuncTypeExpr::get_return_type() const{
    return this->return_type;
}
std::size_t FuncTypeExpr::get_size() const{
    return 0;
}
std::string FuncTypeExpr::to_string() const{
    std::string res = "fn(";
    for(size_t i=0;i<this->param_types.size();i++){
        res += this->param_types[i]->to_string();
        if(i!=this->param_types.size()-1){
            res += ", ";
        }
    }
    res += ") -> " + this->return_type->to_string();
    return res;
}
}
}
}