#include "ir/parser/parser.hpp"
#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <iostream>

namespace Hazel {
namespace Snap{
namespace IR {
Parser::Parser(const std::vector<Token>& toks, const std::string& filename){
    this->toks = toks;
    this->filename = filename;
    if(!toks.empty()){
        this->curr_tok = toks[0];
    }
    parse();
    typecheck();
}
void Parser::parse(){
    std::vector<FunctionPtr> items;
    if(this->toks.empty()){
        this->ast = std::make_shared<Program>(items, std::nullopt, false);
        return;
    }
    while(this->curr_tok.type != TokenType::eof){
        auto item = parse_global_item();
        if(item != nullptr){
            items.push_back(item);
        }
        advance();
    }
    this->ast = std::make_shared<Program>(items, std::nullopt, false);
}
ProgramPtr Parser::get_ast(){
    return this->ast;
}
DebugInfoPtr Parser::parse_debug_info(){
    Token tok = this->curr_tok;//the ! token
    advance();//After the ! token
    if(this->curr_tok.type != TokenType::string && this->curr_tok.type != TokenType::raw_string){
        error(this->curr_tok, "Expected a string literal after '!' for debug info");
    }
    Token file_name = this->curr_tok;
    expect(TokenType::colon, "Expected ':' after file name in debug info");
    expect(TokenType::integer, "Expected a number literal for line number in debug info");
    Token line = this->curr_tok;
    expect(TokenType::colon, "Expected ':' after line number in debug info");
    expect(TokenType::integer, "Expected a number literal for column number in debug info");
    Token column = this->curr_tok;
    return std::make_shared<DebugInfo>(tok, file_name, line, column);
}
InstructionStmtPtr Parser::parse_instruction_stmt(){
    Token tok = this->curr_tok;
    std::optional<std::pair<Token, TypeExprPtr>> name = std::nullopt;
    std::optional<std::string> original_name = std::nullopt;
    if(tok.type == TokenType::kw_let){
        advance();
        TypeExprPtr type = parse_type_expr();
        expect(TokenType::colon, "Expected ':' after type in instruction definition");
        expect(TokenType::local_identifier, "Expected a local identifier for register name");
        Token name_tok = this->curr_tok;
        name = std::make_pair(name_tok, type);
        if(peek().type == TokenType::colon){
            advance();//on the ':' token
            expect(TokenType::string, "Expected a string literal for original register name");
            original_name = this->curr_tok.value;
        }
        expect(TokenType::assign, "Expected '=' after register name in instruction definition");
        advance();//After the '=' token
    }
    if(this->curr_tok.type != TokenType::instruction_identifier){
        error(this->curr_tok, "Expected an instruction");
    }
    Token instr_name = this->curr_tok;
    std::vector<LiteralExprPtr> operands;
    if(peek().type == TokenType::lparen){
        advance();//on the '(' token
        while(peek().type != TokenType::rparen){
            advance();//After `,` or after '(' for the first operand
            operands.push_back(parse_literal_expr(true));
            if(peek().type == TokenType::comma){
                advance();//advance on the ',' token to the next operand
            }
            else if(peek().type != TokenType::rparen){
                error(this->peek(), "Expected ',' or ')' after instruction operand");
            }
        }
        expect(TokenType::rparen, "Expected ')' after instruction operands");
    }
    DebugInfoPtr debug_info = nullptr;
    if(peek().type == TokenType::bang){
        advance();//on the '!' token for debug info
        debug_info = parse_debug_info();
    }
    return std::make_shared<InstructionStmt>(tok, instr_name, name, original_name, operands, debug_info);
}
LabelPtr Parser::parse_label(){
    Token tok = this->curr_tok;//the label token
    expect(TokenType::label_identifier, "Expected a label identifier after 'label'");
    Token name = this->curr_tok;
    std::vector<std::pair<Token, TypeExprPtr>> params;
    if(peek().type == TokenType::lparen){
        advance();//on the '(' token
        while(peek().type != TokenType::rparen){
            expect(TokenType::kw_let, "Expected 'let' for label parameter declaration");//After `,` or after '(' for the first parameter
            advance();
            TypeExprPtr type = parse_type_expr();
            expect(TokenType::colon, "Expected ':' after type in label parameter");
            expect(TokenType::local_identifier, "Expected a local identifier for label parameter name");
            Token param_name = this->curr_tok;
            params.push_back({param_name, type});
            if(peek().type == TokenType::comma){
                advance();//advance on the ',' token to the next parameter
            }
            else if(peek().type != TokenType::rparen){
                error(this->peek(), "Expected ',' or ')' after label parameter");
            }
        }
        expect(TokenType::rparen, "Expected ')' after label parameters");
    }
    DebugInfoPtr debug_info = nullptr;
    if(peek().type == TokenType::bang){
        advance();//on the '!' token for debug info
        debug_info = parse_debug_info();
    }
    std::vector<InstructionStmtPtr> statements;
    expect(TokenType::lbrace, "Expected '{' after label identifier");
    while(peek().type != TokenType::rbrace){
        advance();//advance to the next instruction or label
        statements.push_back(parse_instruction_stmt());
        if(peek().type == TokenType::semicolon){
            advance();//advance on the ';' token to the next instruction or label
        }
        else if(peek().type != TokenType::rbrace){
            error(this->peek(), "Expected ';' or '}' after instruction in label");
        }
    }
    expect(TokenType::rbrace, "Expected '}' after label statements");
    return std::make_shared<Label>(tok, name, statements, params, debug_info, false);
}
std::vector<LabelPtr> Parser::parse_labels(){
    std::vector<LabelPtr> labels;
    while(this->curr_tok.type == TokenType::kw_label){
        labels.push_back(parse_label());
        advance_on_semicolon();
        if(peek().type == TokenType::kw_label){
            advance();//advance to the next label
        }
    }
    return labels;
}
FunctionPtr Parser::parse_function(){
    Token tok = this->curr_tok;
    expect(TokenType::global_func_identifier, "Expected a global identifier for function name");
    Token name = this->curr_tok;
    expect(TokenType::lparen, "Expected '(' after function name");
    std::vector<std::pair<Token, TypeExprPtr>> params;
    while(peek().type != TokenType::rparen){
        advance();//After `,` or after '(' for the first parameter
        if(this->curr_tok.type != TokenType::kw_let){
            error(this->curr_tok, "Expected 'let' for function parameter declaration");
        }
        advance();
        TypeExprPtr type = parse_type_expr();
        expect(TokenType::colon, "Expected ':' after type in function parameter");
        expect(TokenType::local_identifier, "Expected a local identifier for function parameter name");
        Token param_name = this->curr_tok;
        params.push_back({param_name, type});
        if(peek().type == TokenType::comma){
            advance();//advance on the ',' token to the next parameter
        }
        else if(peek().type != TokenType::rparen){
            error(this->peek(), "Expected ',' or ')' after function parameter");
        }
    }
    expect(TokenType::rparen, "Expected ')' after function parameters");
    TypeExprPtr return_type = std::make_shared<VoidTypeExpr>();//default return type is void
    if(peek().type == TokenType::arrow){
        advance();//on the '->' token
        advance();//After the '->' token
        return_type = parse_type_expr();
    }
    DebugInfoPtr debug_info = nullptr;
    if(peek().type == TokenType::bang){
        advance();//on the '!' token for debug info
        debug_info = parse_debug_info();
    }
    std::vector<LabelPtr> body;
    if(peek().type == TokenType::lbrace){
        advance();//on the '{' token for function body
        advance_on_semicolon();
        expect(TokenType::kw_label, "Expected a label identifier for the first label in function body");
        body = parse_labels();
        expect(TokenType::rbrace, "Expected '}' after function body");
    }
    return std::make_shared<Function>(tok, name, params, return_type, body, debug_info, false);
}

FunctionPtr Parser::parse_global_item(){
    FunctionPtr item;
    if(this->curr_tok.type == TokenType::kw_fn){
        item = parse_function();
        advance_on_semicolon();
    }
    else if(this->curr_tok.type == TokenType::kw_let){
        expect(TokenType::type_identifier, "Expected a type identifier for global type declaration");
        auto type_name = this->curr_tok;
        expect(TokenType::assign, "Expected '=' after type identifier in global type declaration");
        advance();//After the '=' token
        auto type_expr = parse_type_expr();
        this->global_typedef.push_back({type_name, type_expr});
        expect(TokenType::semicolon, "Expected ';' after global variable declaration");
    }
    else if(this->curr_tok.type == TokenType::eof || this->curr_tok.type == TokenType::semicolon){
        //allow empty global items to simplify parsing. We will just ignore them in the AST.
        item = nullptr;
    }
    else{
        error(this->curr_tok, "Expected a function, global variable declaration or global attribute declaration");
    }
    return item;
}
}
}
}