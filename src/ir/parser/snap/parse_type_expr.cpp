#include "ir/lexer/token.hpp"
#include "ir/parser/parser.hpp"
#include <algorithm>
#include <iostream>

namespace Hazel {
namespace Snap{
namespace IR {
TypeExprPtr Parser::parse_type_expr(){
    switch (this->curr_tok.type){
        case TokenType::type_identifier:{
            return std::make_shared<NamedTypeExpr>(this->curr_tok);
        }
        case TokenType::kw_integer:{
            return std::make_shared<IntTypeExpr>(this->curr_tok);
        }
        case TokenType::kw_decimal:{
            return std::make_shared<DecimalTypeExpr>(this->curr_tok);
        }
        case TokenType::kw_string:{
            return std::make_shared<StringTypeExpr>(this->curr_tok);
        }
        case TokenType::kw_ptr:{
            return parse_ptr_type_expr();
        }
        case TokenType::lbracket:{
            return parse_array_type_expr();
        }
        case TokenType::langel:{   
            return parse_packed_struct_type_expr();
        }
        case TokenType::lbrace:{
            return parse_struct_type_expr();
        }
        case TokenType::kw_fn:{
            return parse_func_type_expr();
        }
        case TokenType::kw_label:{
            return parse_label_type_expr();
        }
        default:{
            std::cout << "Unexpected token type: " << this->curr_tok << std::endl;
            error(this->curr_tok, "Expected a type expression");
        }
    }
}
TypeExprPtr Parser::parse_ptr_type_expr(){
    Token tok = this->curr_tok;//the 'ptr' token
    expect(TokenType::langel, "Expected '<' after 'ptr' in pointer type expression");
    expect(TokenType::integer, "Expected pointer element size after '<' in pointer type expression");
    std::uint64_t element_size;
    try{
        element_size = Utils::to_numeric<std::uint64_t>(this->curr_tok.value, true, true);
    }
    catch(const std::exception& e){
        error(this->curr_tok, "Invalid pointer element size in pointer type expression", "Expected a positive integer");
    }
    expect(TokenType::rangel, "Expected '>' after pointer element size in pointer type expression");
    return std::make_shared<PtrTypeExpr>(tok, element_size);
}
TypeExprPtr Parser::parse_array_type_expr(){
    Token tok = this->curr_tok;//the [ token
    advance();//After the [ token
    TypeExprPtr base_type = parse_type_expr();
    expect(TokenType::rbracket, "Expected ']' after array size in array type expression");
    return std::make_shared<ArrayTypeExpr>(tok, base_type);
}
TypeExprPtr Parser::parse_packed_struct_type_expr(){
    Token tok = this->curr_tok;//the <
    expect(TokenType::lbrace, "Expected '{' after '<' in packed struct type expression");
    std::vector<TypeExprPtr> fields;
    while(peek().type != TokenType::rbrace){
        advance();//After the { token or the , token
        fields.push_back(parse_type_expr());
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rbrace){
            error(peek(), "Expected ',' or '}' after struct type field in struct type expression");
        }
    }
    expect(TokenType::rbrace, "Expected '}' after struct type fields in struct type expression");
    expect(TokenType::rangel, "Expected '>' after packed struct type fields in packed struct type expression");
    return std::make_shared<StructTypeExpr>(tok, fields, true);
}
TypeExprPtr Parser::parse_struct_type_expr(){
    Token tok = this->curr_tok;//the { 
    std::vector<TypeExprPtr> fields;
    while(peek().type != TokenType::rbrace){
        advance();//After the { token or the , token
        fields.push_back(parse_type_expr());
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rbrace){
            error(peek(), "Expected ',' or '}' after struct type field in struct type expression");
        }
    }
    expect(TokenType::rbrace, "Expected '}' after struct type fields in struct type expression");
    return std::make_shared<StructTypeExpr>(tok, fields, false);
}
TypeExprPtr Parser::parse_func_type_expr(){
    // fn(type1, type2, ... ) -> return_type
    Token tok = this->curr_tok;//the fn token
    expect(TokenType::lparen, "Expected '(' after 'fn' in function type expression");
    std::vector<TypeExprPtr> param_types;
    while(peek().type != TokenType::rparen){
        advance();//After the ( token or the , token
        param_types.push_back(parse_type_expr());
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rparen){
            error(peek(), "Expected ',' or ')' after function type parameter in function type expression");
        }
    }
    expect(TokenType::rparen, "Expected ')' after function type parameters in function type expression");
    TypeExprPtr return_type = std::make_shared<VoidTypeExpr>();
    if(peek().type == TokenType::arrow){
        advance();//On the -> token
        return_type = parse_type_expr();
    }
    return std::make_shared<FuncTypeExpr>(tok, param_types, return_type);
}

TypeExprPtr Parser::parse_label_type_expr(){
    Token tok = this->curr_tok;//the label token
    std::vector<TypeExprPtr> params;
    if(peek().type == TokenType::lparen){
        advance();//on the '(' token
        while(peek().type != TokenType::rparen){
            advance();//After the ( token or the , token
            params.push_back(parse_type_expr());
            if(peek().type == TokenType::comma){
                advance();//On the , token
            }
            else if(peek().type != TokenType::rparen){
                error(peek(), "Expected ',' or ')' after label type parameter in label type expression");
            }
        }
        expect(TokenType::rparen, "Expected ')' after label type parameters in label type expression");
    }
    return std::make_shared<LabelTypeExpr>(tok, params);
}
}
}
}