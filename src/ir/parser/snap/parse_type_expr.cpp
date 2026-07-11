#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include <algorithm>
#include <iostream>

namespace LIRA{
namespace IR{
TypeExprPtr Parser::parse_type_expr(bool has_attribute){
    switch (this->curr_tok.type){
        case TokenType::global_identifier:
        case TokenType::local_identifier:
        case TokenType::builtin_identifier:{
            return parse_named_type_expr(has_attribute);
        }
        case TokenType::kw_scope:{
            return std::make_shared<ScopeTypeExpr>(this->curr_tok);
        }
        case TokenType::lparen:{
            advance();//After '(' 
            TypeExprPtr stmt = parse_type_expr(true);
            expect(TokenType::rparen, "Expected ')' after type expression");
            return stmt;
        }
        case TokenType::lbracket:{
            return parse_array_type_expr(has_attribute);
        }
        case TokenType::langel:{   
            if(peek().type == TokenType::lbrace){
                return parse_struct_type_expr(has_attribute);
            }
            return parse_simd_type_expr(has_attribute);
        }
        case TokenType::lbrace:{
            return parse_struct_type_expr(has_attribute);
        }
        case TokenType::kw_fn:{
            return parse_func_type_expr(has_attribute);
        }
        case TokenType::kw_label:{
            return parse_label_type_expr(has_attribute);
        }
        default:{
            std::cout << "Unexpected token type: " << this->curr_tok << std::endl;
            error(this->curr_tok, "Expected a type expression");
        }
    }
}
TypeExprPtr Parser::parse_named_type_expr(bool has_attribute){
    Token name = this->curr_tok;
    std::vector<AttributePtr> attributes;
    if(has_attribute && peek().type == TokenType::hash){
        advance();//On the # token
        attributes = parse_attributes();
    }
    if(name.value == "void"){
        return std::make_shared<VoidTypeExpr>(name, attributes);
    }
    else if(name.value == "ptr"){
        return std::make_shared<PtrTypeExpr>(name, attributes);
    }
    else if(name.value == "type"){
        return std::make_shared<MetaTypeExpr>(name, attributes);
    }
    else if(name.value == "f16" || name.value == "bf16" || name.value == "f32" || name.value == "f64" || name.value == "f80" || name.value == "f128"){
        bool is_brain_float = name.value == "bf16";
        std::size_t bits = 0;
        if(name.value[0] == 'f'){
            bits = Utils::to_numeric<std::size_t>(name.value.substr(1));
        }
        else{
            //Happens only for bf16
            bits = 16;
        }
        return std::make_shared<FloatTypeExpr>(name, bits, is_brain_float, attributes);
    }
    else if(name.value[0] == 'i' && name.value.length() > 1){
        try{
            std::size_t bits = Utils::to_numeric<std::size_t>(name.value.substr(1));
            return std::make_shared<IntTypeExpr>(name, bits, attributes);
        }
        catch(const std::exception& e){
            return std::make_shared<NamedTypeExpr>(name, attributes);
        }
    }
    
    return std::make_shared<NamedTypeExpr>(name, attributes);
}
TypeExprPtr Parser::parse_array_type_expr(bool has_attribute){
    Token tok = this->curr_tok;//the [ token
    advance();//After the [ token
    TypeExprPtr base_type = parse_type_expr(true);
    expect(TokenType::comma, "Expected ',' after base type in array type expression");
    expect(TokenType::number, "Expected array size after '[' in array type expression");
    std::size_t size;
    try{
        size = Utils::to_numeric<std::size_t>(this->curr_tok.value, true, true);
    }
    catch(const std::exception& e){
        error(this->curr_tok, "Invalid array size in array type expression", "Expected a positive integer");
    }
    expect(TokenType::rbracket, "Expected ']' after array size in array type expression");
    std::vector<AttributePtr> attributes;
    if(has_attribute && peek().type == TokenType::hash){
        advance();//On the # token
        attributes = parse_attributes();
    }
    return std::make_shared<ArrayTypeExpr>(tok, base_type, size, attributes);
}
TypeExprPtr Parser::parse_simd_type_expr(bool has_attribute){
    Token tok = this->curr_tok;//the < token
    advance();//After the < token
    TypeExprPtr base_type = parse_type_expr(true);
    expect(TokenType::comma, "Expected ',' after base type in SIMD type expression");
    expect(TokenType::number, "Expected SIMD size after '<' in SIMD type expression");
    std::size_t size;
    try{
        size = Utils::to_numeric<std::size_t>(this->curr_tok.value, true, true);
    }
    catch(const std::exception& e){
        error(this->curr_tok, "Invalid SIMD size in SIMD type expression", "Expected a positive integer");
    }
    expect(TokenType::rangel, "Expected '>' after SIMD size in SIMD type expression");
    std::vector<AttributePtr> attributes;
    if(has_attribute && peek().type == TokenType::hash){
        advance();//On the # token
        attributes = parse_attributes();
    }
    return std::make_shared<SIMDTypeExpr>(tok, base_type, size, attributes);
}
TypeExprPtr Parser::parse_struct_type_expr(bool has_attribute){
    bool is_packed = false;
    Token tok = this->curr_tok;//the { or < token
    std::vector<TypeExprPtr> fields;
    if(this->curr_tok.type == TokenType::langel){
        is_packed = true;
        advance();//After the < token
    }
    // {type1, type2, ...} #[attributes]
    while(peek().type != TokenType::rbrace){
        advance();//After the { token or the , token
        fields.push_back(parse_type_expr(true));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rbrace){
            error(peek(), "Expected ',' or '}' after struct type field in struct type expression");
        }
    }
    expect(TokenType::rbrace, "Expected '}' after struct type fields in struct type expression");
    if(is_packed){
        expect(TokenType::rangel, "Expected '>' after packed struct type fields in struct type expression");
    }
    std::vector<AttributePtr> attributes;
    if(has_attribute && peek().type == TokenType::hash){
        advance();//On the # token
        attributes = parse_attributes();
    }
    return std::make_shared<StructTypeExpr>(tok, fields, is_packed, attributes);
}
TypeExprPtr Parser::parse_func_type_expr(bool has_attribute){
    // fn(type1, type2, ... ) -> return_type #[attributes]
    Token tok = this->curr_tok;//the fn token
    expect(TokenType::lparen, "Expected '(' after 'fn' in function type expression");
    std::vector<TypeExprPtr> param_types;
    bool varargs = false;
    while(peek().type != TokenType::rparen){
        advance();//After the ( token or the , token
        if(this->curr_tok.type == TokenType::ellipsis){
            varargs = true;
            break;
        }
        param_types.push_back(parse_type_expr(true));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rparen){
            error(peek(), "Expected ',' or ')' after function type parameter in function type expression");
        }
    }
    expect(TokenType::rparen, "Expected ')' after function type parameters in function type expression");
    expect(TokenType::arrow, "Expected '->' after function type parameters in function type expression");
    advance();//after the '->' token
    TypeExprPtr return_type = parse_type_expr(false);//Put expr within bracket to only allow attributes on the return type
    std::vector<AttributePtr> attributes;
    if(has_attribute && peek().type == TokenType::hash){
        advance();//On the # token
        attributes = parse_attributes();
    }
    return std::make_shared<FuncTypeExpr>(tok, param_types, varargs, return_type, attributes);
}
TypeExprPtr Parser::parse_label_type_expr(bool has_attribute){
    Token tok = this->curr_tok;//the label token
    std::vector<TypeExprPtr> params;
    if(peek().type == TokenType::lparen){
        advance();//on the '(' token
        while(peek().type != TokenType::rparen){
            advance();//After the ( token or the , token
            params.push_back(parse_type_expr(true));
            if(peek().type == TokenType::comma){
                advance();//On the , token
            }
            else if(peek().type != TokenType::rparen){
                error(peek(), "Expected ',' or ')' after label type parameter in label type expression");
            }
        }
        expect(TokenType::rparen, "Expected ')' after label type parameters in label type expression");
    }
    std::vector<AttributePtr> attributes;
    if(has_attribute && peek().type == TokenType::hash){
        advance();//On the # token
        attributes = parse_attributes();
    }
    return std::make_shared<LabelTypeExpr>(tok, params, attributes);
}
}
}