#include "ir/parser/parser.hpp"

namespace Hazel {
namespace Snap{
namespace IR {
LiteralExprPtr Parser::parse_literal_expr(bool parse_type){
    TypeExprPtr type = nullptr;
    if(parse_type){
        type = parse_type_expr();
        expect(TokenType::colon, "Expected ':' after type in literal expression");
        advance();//After the : token
    }
    switch (this->curr_tok.type){
        case TokenType::kw_zeroinit:{
            return std::make_shared<ZeroInitLiteralExpr>(this->curr_tok, type);
        }
        case TokenType::kw_poison:{
            return std::make_shared<PoisonLiteralExpr>(this->curr_tok, type);
        }
        case TokenType::local_identifier:{
            return std::make_shared<NamedLiteralExpr>(this->curr_tok, type);
        }
        case TokenType::integer:
        case TokenType::decimal:{
            return std::make_shared<NumLiteralExpr>(this->curr_tok, type);
        }
        case TokenType::string:
        case TokenType::raw_string:{
            return std::make_shared<StringLiteralExpr>(this->curr_tok, type);
        }
        case TokenType::lbracket:{
            return parse_array_literal_expr(type);
        }
        case TokenType::langel:{
            if(peek().type == TokenType::lbracket){
                return parse_packed_array_literal_expr(type);
            }
            else{
                return parse_packed_struct_literal_expr(type);
            }
            error(peek(), "Expected '[' or '{' after '<' in packed literal expression");
        }
        case TokenType::lbrace:{
            return parse_struct_literal_expr(type);
        }
        case TokenType::label_identifier:{
            return parse_label_literal_expr(type);
        }
        case TokenType::global_func_identifier:
        case TokenType::extern_func_identifier:{
            return parse_func_literal_expr(type);
        }
        default:{
            error(curr_tok, "Expected a literal expression");
        }
    }
}
LiteralExprPtr Parser::parse_packed_array_literal_expr(TypeExprPtr type){
    Token tok = this->curr_tok;//the < token
    expect(TokenType::lbracket, "Expected '[' after '<' in packed array literal expression");
    std::vector<LiteralExprPtr> elements;
    while(peek().type != TokenType::rbracket){
        advance();//After the [ token or the , token
        elements.push_back(parse_literal_expr(false));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rbracket){
            error(peek(), "Expected ',' or ']' after array literal element in array literal expression");
        }
    }
    expect(TokenType::rbracket, "Expected ']' after array literal elements in array literal expression");
    expect(TokenType::rangel, "Expected '>' after packed array literal elements in packed array literal expression");
    return std::make_shared<ArrayLiteralExpr>(tok, elements, true, type);
}
LiteralExprPtr Parser::parse_array_literal_expr(TypeExprPtr type){
    Token tok = this->curr_tok;//the [ token
    std::vector<LiteralExprPtr> elements;
    while(peek().type != TokenType::rbracket){
        advance();//After the [ token or the , token
        elements.push_back(parse_literal_expr(false));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rbracket){
            error(peek(), "Expected ',' or ']' after array literal element in array literal expression");
        }
    }
    expect(TokenType::rbracket, "Expected ']' after array literal elements in array literal expression");
    return std::make_shared<ArrayLiteralExpr>(tok, elements, false, type);
}
LiteralExprPtr Parser::parse_packed_struct_literal_expr(TypeExprPtr type){
    Token tok = this->curr_tok;//the < token
    expect(TokenType::lbrace, "Expected '{' after '<' in packed struct literal expression");
    std::vector<LiteralExprPtr> elements;
    while(peek().type != TokenType::rbrace){
        advance();//After the { token or the , token
        elements.push_back(parse_literal_expr(false));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rbrace){
            error(peek(), "Expected ',' or '}' after struct literal element in struct literal expression");
        }
    }
    expect(TokenType::rbrace, "Expected '}' after struct literal elements in struct literal expression");
    expect(TokenType::rangel, "Expected '>' after packed struct literal elements in packed struct literal expression");
    return std::make_shared<StructLiteralExpr>(tok, elements, true, type);
}
LiteralExprPtr Parser::parse_struct_literal_expr(TypeExprPtr type){
    Token tok = this->curr_tok;//the { token
    std::vector<LiteralExprPtr> elements;
    while(peek().type != TokenType::rbrace){
        advance();//After the { token or the , token
        elements.push_back(parse_literal_expr(false));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rbrace){
            error(peek(), "Expected ',' or '}' after struct literal element in struct literal expression");
        }
    }
    expect(TokenType::rbrace, "Expected '}' after struct literal elements in struct literal expression");
    return std::make_shared<StructLiteralExpr>(tok, elements, false, type);
}
LiteralExprPtr Parser::parse_label_literal_expr(TypeExprPtr type){
    Token tok = this->curr_tok;//the label identifier token
    if(peek().type != TokenType::lparen){
        return std::make_shared<LabelLiteralExpr>(tok, std::vector<LiteralExprPtr>(), type);
    }
    expect(TokenType::lparen, "Expected '(' after label identifier in label literal expression");
    std::vector<LiteralExprPtr> args;
    while(peek().type != TokenType::rparen){
        advance();//After the ( token or the , token
        args.push_back(parse_literal_expr(false));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rparen){
            error(peek(), "Expected ',' or ')' after label literal argument in label literal expression");
        }
    }
    expect(TokenType::rparen, "Expected ')' after label literal arguments in label literal expression");
    return std::make_shared<LabelLiteralExpr>(tok, args, type);
}
LiteralExprPtr Parser::parse_func_literal_expr(TypeExprPtr type){
    Token tok = this->curr_tok;//the function identifier token
    if(peek().type != TokenType::lparen){
        return std::make_shared<FuncLiteralExpr>(tok, std::vector<LiteralExprPtr>(), type);
    }
    expect(TokenType::lparen, "Expected '(' after function identifier in function literal expression");
    std::vector<LiteralExprPtr> args;
    while(peek().type != TokenType::rparen){
        advance();//After the ( token or the , token
        args.push_back(parse_literal_expr(false));
        if(peek().type == TokenType::comma){
            advance();//On the , token
        }
        else if(peek().type != TokenType::rparen){
            error(peek(), "Expected ',' or ')' after function literal argument in function literal expression");
        }
    }
    expect(TokenType::rparen, "Expected ')' after function literal arguments in function literal expression");
    return std::make_shared<FuncLiteralExpr>(tok, args, type);
}
}
}
}