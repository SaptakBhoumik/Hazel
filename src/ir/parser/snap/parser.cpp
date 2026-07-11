#include "parser/parser.hpp"
#include "ast/ast.hpp"
#include "lexer/token.hpp"
#include <iostream>

namespace LIRA {
namespace IR {
Parser::Parser(const std::vector<Token>& toks, const std::string& filename){
    this->toks = toks;
    this->filename = filename;
    if(!toks.empty()){
        this->curr_tok = toks[0];
    }
}
ProgramPtr Parser::parse(){
    std::vector<GlobalItemPtr> items;
    if(this->toks.empty()){
        return std::make_shared<Program>(items);
    }
    while(this->curr_tok.type != TokenType::eof){
        auto item = parse_global_item();
        if(item != nullptr){
            items.push_back(item);
        }
        advance();
    }
    return std::make_shared<Program>(items);
}
DebugInfoPtr Parser::parse_debug_info(){
    Token tok = this->curr_tok;//the ! token
    advance();//After the ! token
    if(this->curr_tok.type != TokenType::string && this->curr_tok.type != TokenType::raw_string){
        error(this->curr_tok, "Expected a string literal after '!' for debug info");
    }
    Token file_name = this->curr_tok;
    expect(TokenType::colon, "Expected ':' after file name in debug info");
    expect(TokenType::number, "Expected a number literal for line number in debug info");
    Token line = this->curr_tok;
    expect(TokenType::colon, "Expected ':' after line number in debug info");
    expect(TokenType::number, "Expected a number literal for column number in debug info");
    Token column = this->curr_tok;
    return std::make_shared<DebugInfo>(tok, file_name, line, column);
}
std::pair<ExprPtr,TypeExprPtr> Parser::parse_type_expr_pair(){
    bool is_type_expr = this->curr_tok.value == "type";
    TypeExprPtr type = parse_type_expr(true);
    expect(TokenType::colon, "Expected ':' after type in type:expr pair");
    advance();
    ExprPtr expr;
    if(is_type_expr){
        expr = std::make_shared<Expr>(parse_type_expr(true));
    }
    else{
        expr = std::make_shared<Expr>(parse_literal_expr());
    }
    return {expr, type};
}

AttributePtr Parser::parse_attribute(){
    Token tok = this->curr_tok;//the # token
    expect(TokenType::lbracket, "Expected '[' after '#' for attribute");
    expect(TokenType::builtin_identifier, "Expected a buildin identifier after '#' for attribute name");
    Token name = this->curr_tok;
    std::vector<std::pair<ExprPtr,TypeExprPtr>> args;
    std::vector<Utils::triplet<Token,ExprPtr,TypeExprPtr>> kwargs;
    if(this->peek().type == TokenType::lparen){
        advance();//on '('
        while(peek().type != TokenType::rparen){
            advance();//After `,` or after '(' for the first arg
            if(this->curr_tok.type == TokenType::builtin_identifier && this->peek().type == TokenType::assign){
                //kwarg
                Token kwarg_name = this->curr_tok;
                advance();
                advance();
                auto [expr, type] = parse_type_expr_pair();
                kwargs.push_back({kwarg_name, expr, type});
            }
            else{
                if(kwargs.size() > 0){
                    error(this->curr_tok, "Positional arguments cannot come after keyword arguments in attribute");
                }
                //arg
                args.push_back(parse_type_expr_pair());
            }
            if(peek().type == TokenType::comma){
                advance();//advance on the ',' token to the next arg/kwarg
            }
            else if(peek().type != TokenType::rparen){
                error(this->peek(), "Expected ',' or ')' after attribute argument");
            }
        }
        expect(TokenType::rparen, "Expected ')' after attribute arguments");
    }
    expect(TokenType::rbracket, "Expected ']' after attribute");
    return std::make_shared<Attribute>(tok, name, args, kwargs);
}

std::vector<AttributePtr> Parser::parse_attributes(){
    std::vector<AttributePtr> attributes;
    while(this->curr_tok.type == TokenType::hash){
        attributes.push_back(parse_attribute());
        if(peek().type == TokenType::hash){
            advance();//advance on the '#' token to the next attribute
        }
    }
    return attributes;
}

InstructionStmtPtr Parser::parse_instruction_stmt(bool is_global){
    if(this->curr_tok.type != TokenType::kw_let && is_global){
        error(this->curr_tok, "Expected 'let' for global variable declaration");
    }
    Token tok = this->curr_tok;
    std::optional<Utils::triplet<Token, std::vector<AttributePtr>, TypeExprPtr>> name;
    if(tok.type == TokenType::kw_let){
        advance();
        TypeExprPtr type = parse_type_expr(true);
        expect(TokenType::colon, "Expected ':' after type in instruction definition");
        if(is_global){
            expect(TokenType::global_identifier, "Expected a global identifier for register name");
        }
        else{
            expect(TokenType::local_identifier, "Expected a local identifier for register name");
        }
        Token name_tok = this->curr_tok;
        if(peek().type == TokenType::hash){
            advance();//advance on the '#' token to the first attribute
            name = Utils::triplet(name_tok, parse_attributes(), type);
        }
        else{
            name = Utils::triplet(name_tok, std::vector<AttributePtr>{}, type);
        }
        if(peek().type == TokenType::assign){
            advance();//on the '=' token
            advance();//After the '=' token
        }
        else{
            return std::make_shared<InstructionStmt>(tok, name, std::nullopt);
        }
    }
    if(this->curr_tok.type != TokenType::instruction_identifier){
        error(this->curr_tok, "Expected an instruction");
    }
    Token instr_name = this->curr_tok;
    std::vector<std::pair<ExprPtr,TypeExprPtr>> operands;
    if(peek().type == TokenType::lparen){
        advance();//on the '(' token
        while(peek().type != TokenType::rparen){
            advance();//After `,` or after '(' for the first operand
            operands.push_back(parse_type_expr_pair());
            if(peek().type == TokenType::comma){
                advance();//advance on the ',' token to the next operand
            }
            else if(peek().type != TokenType::rparen){
                error(this->peek(), "Expected ',' or ')' after instruction operand");
            }
        }
        expect(TokenType::rparen, "Expected ')' after instruction operands");
    }
    std::vector<AttributePtr> attributes;
    DebugInfoPtr debug_info = nullptr;
    if(peek().type == TokenType::hash){
        advance();//advance on the '#' token to the first attribute
        attributes = parse_attributes();
    }
    if(peek().type == TokenType::bang){
        advance();//on the '!' token for debug info
        debug_info = parse_debug_info();
    }
    std::optional<InstructionStmt::InstructionCall> value = InstructionStmt::InstructionCall(instr_name, operands, attributes, debug_info);
    return std::make_shared<InstructionStmt>(tok, name, value);
}

ScopePtr Parser::parse_scope(){
    Token tok = this->curr_tok;//the 'scope' token
    expect(TokenType::scope_identifier, "Expected a scope identifier after 'scope'");
    Token scope_var_name = this->curr_tok;
    expect(TokenType::assign, "Expected '=' after scope variable name");
    expect(TokenType::builtin_identifier, "Expected a identifier for scope type");
    ScopeType scope_type;
    if(this->curr_tok.value == "function"){
        scope_type = ScopeType::FunctionScope;
    }
    else if(this->curr_tok.value == "block"){
        scope_type = ScopeType::BlockScope;
    }
    else if(this->curr_tok.value == "inline"){
        scope_type = ScopeType::InlineScope;
    }
    else{
        error(this->curr_tok, "Expected 'function', 'block' or 'inline' for scope type");
    }
    std::optional<Token> scope_name = std::nullopt;
    std::optional<Token> parent_scope_name;
    std::optional<Utils::triplet<Token,Token,Token>> scope_loc;
    std::optional<Utils::triplet<Token,Token,Token>> callsite_loc;
    if(peek().type == TokenType::lparen){
        advance();//on the '(' token
        while(peek().type != TokenType::rparen){
            advance();
            if(this->curr_tok.value == "scope_name"){
                if(scope_name.has_value()){
                    error(this->curr_tok, "Duplicate 'scope_name' in scope definition");
                }
                expect(TokenType::assign, "Expected '=' after 'scope_name' in scope definition");
                advance();
                if(this->curr_tok.type != TokenType::string && this->curr_tok.type != TokenType::raw_string){
                    error(this->curr_tok, "Expected a string literal for scope name in scope_name");
                }
                scope_name = this->curr_tok;
            }
            else if(this->curr_tok.value == "parent_scope"){
                if(parent_scope_name.has_value()){
                    error(this->curr_tok, "Duplicate 'parent_scope' in scope definition");
                }
                expect(TokenType::assign, "Expected '=' after 'parent_scope' in scope definition");
                expect(TokenType::scope_identifier, "Expected a scope identifier for parent scope name");
                parent_scope_name = this->curr_tok;
            }
            else if(this->curr_tok.value == "scope_loc"){
                if(scope_loc.has_value()){
                    error(this->curr_tok, "Duplicate 'scope_loc' in scope definition");
                }
                expect(TokenType::assign, "Expected '=' after 'scope_loc' in scope definition");
                advance();
                Token file_name = this->curr_tok;
                if(file_name.type != TokenType::string && file_name.type != TokenType::raw_string){
                    error(this->curr_tok, "Expected a string literal for file name in scope_loc");
                }
                expect(TokenType::colon, "Expected ':' after file name in scope_loc");
                expect(TokenType::number, "Expected a number literal for line number in scope_loc");
                Token line = this->curr_tok;
                expect(TokenType::colon, "Expected ':' after line number in scope_loc");
                expect(TokenType::number, "Expected a number literal for column number in scope_loc");
                scope_loc = Utils::triplet(file_name, line, this->curr_tok);
            }
            else if(this->curr_tok.value == "callsite_loc"){
                if(callsite_loc.has_value()){
                    error(this->curr_tok, "Duplicate 'callsite_loc' in scope definition");
                }
                expect(TokenType::assign, "Expected '=' after 'callsite_loc' in scope definition");
                advance();
                Token file_name = this->curr_tok;
                if(file_name.type != TokenType::string && file_name.type != TokenType::raw_string){
                    error(this->curr_tok, "Expected a string literal for file name in callsite_loc");
                }
                expect(TokenType::colon, "Expected ':' after file name in callsite_loc");
                expect(TokenType::number, "Expected a number literal for line number in callsite_loc");
                Token line = this->curr_tok;
                expect(TokenType::colon, "Expected ':' after line number in callsite_loc");
                expect(TokenType::number, "Expected a number literal for column number in callsite_loc");
                callsite_loc = Utils::triplet(file_name, line, this->curr_tok);
            }
            else{
                error(this->curr_tok, "Unexpected keyword in scope definition. Expected 'scope_name', 'parent_scope', 'scope_loc' or 'callsite_loc'");
            }

            if(peek().type == TokenType::comma){
                advance();//advance on the ',' token to the next parameter
            }
            else if(peek().type != TokenType::rparen){
                error(this->peek(), "Expected ',' or ')' after scope parameter");
            }
        }
        expect(TokenType::rparen, "Expected you to close the '('");
    }
    return std::make_shared<Scope>(tok, scope_type, scope_name, scope_var_name, parent_scope_name, scope_loc, callsite_loc);
}
LabelPtr Parser::parse_label(){
    Token tok = this->curr_tok;//the label token
    expect(TokenType::label_identifier, "Expected a label identifier after 'label'");
    Token name = this->curr_tok;
    std::vector<Utils::triplet<Token, std::vector<AttributePtr>, TypeExprPtr>> params;
    if(peek().type == TokenType::lparen){
        advance();//on the '(' token
        while(peek().type != TokenType::rparen){
            expect(TokenType::kw_let, "Expected 'let' for label parameter declaration");//After `,` or after '(' for the first parameter
            advance();
            TypeExprPtr type = parse_type_expr(true);
            expect(TokenType::colon, "Expected ':' after type in label parameter");
            expect(TokenType::local_identifier, "Expected a local identifier for label parameter name");
            Token param_name = this->curr_tok;
            std::vector<AttributePtr> param_attributes;
            if(peek().type == TokenType::hash){
                advance();//advance on the '#' token to the first attribute
                param_attributes = parse_attributes();
            }
            params.push_back({param_name, param_attributes, type});
            if(peek().type == TokenType::comma){
                advance();//advance on the ',' token to the next parameter
            }
            else if(peek().type != TokenType::rparen){
                error(this->peek(), "Expected ',' or ')' after label parameter");
            }
        }
        expect(TokenType::rparen, "Expected ')' after label parameters");
    }
    std::vector<InstructionStmtPtr> statements;
    expect(TokenType::lbrace, "Expected '{' after label identifier");
    while(peek().type != TokenType::rbrace){
        advance();//advance to the next instruction or label
        statements.push_back(parse_instruction_stmt(false));
        if(peek().type == TokenType::semicolon){
            advance();//advance on the ';' token to the next instruction or label
        }
        else if(peek().type != TokenType::rbrace){
            error(this->peek(), "Expected ';' or '}' after instruction in label");
        }
    }
    expect(TokenType::rbrace, "Expected '}' after label statements");
    return std::make_shared<Label>(tok, name, statements, params);
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

std::vector<ScopePtr> Parser::parse_scopes(){
    std::vector<ScopePtr> scopes;
    while(this->curr_tok.type == TokenType::kw_scope){
        scopes.push_back(parse_scope());
        expect(TokenType::semicolon, "Expected ';' after scope declaration");
        if(peek().type == TokenType::kw_scope){
            advance();//advance to the next scope
        }
    }
    return scopes;
}
FunctionPtr Parser::parse_function(){
    Token tok = this->curr_tok;
    std::vector<AttributePtr> attributes;
    if(peek().type == TokenType::hash){
        advance();//advance on the '#' token to the first attribute
        attributes = parse_attributes();
    }
    expect(TokenType::global_identifier, "Expected a global identifier for function name");
    Token name = this->curr_tok;
    expect(TokenType::lparen, "Expected '(' after function name");
    std::vector<Utils::triplet<Token, std::vector<AttributePtr>, TypeExprPtr>> params;
    bool varargs = false;
    while(peek().type != TokenType::rparen){
        advance();//After `,` or after '(' for the first parameter
        if(this->curr_tok.type == TokenType::ellipsis){
            varargs = true;
            break;
        }
        if(this->curr_tok.type != TokenType::kw_let){
            error(this->curr_tok, "Expected 'let' for function parameter declaration");
        }
        advance();
        TypeExprPtr type = parse_type_expr(true);
        expect(TokenType::colon, "Expected ':' after type in function parameter");
        expect(TokenType::local_identifier, "Expected a local identifier for function parameter name");
        Token param_name = this->curr_tok;
        std::vector<AttributePtr> param_attributes;
        if(peek().type == TokenType::hash){
            advance();//advance on the '#' token to the first attribute
            param_attributes = parse_attributes();
        }
        params.push_back({param_name, param_attributes, type});
        if(peek().type == TokenType::comma){
            advance();//advance on the ',' token to the next parameter
        }
        else if(peek().type != TokenType::rparen){
            error(this->peek(), "Expected ',' or ')' after function parameter");
        }
    }
    expect(TokenType::rparen, "Expected ')' after function parameters");
    expect(TokenType::arrow, "Expected '->' after function parameters");
    advance();//after the '->' token
    TypeExprPtr return_type = parse_type_expr(true);
    DebugInfoPtr debug_info = nullptr;
    if(peek().type == TokenType::bang){
        advance();//on the '!' token for debug info
        debug_info = parse_debug_info();
    }
    std::vector<ScopePtr> scopes;
    std::vector<LabelPtr> body;
    if(peek().type == TokenType::lbrace){
        advance();//on the '{' token for function body
        advance_on_semicolon();
        if(peek().type == TokenType::kw_scope){
            advance();//advance to the first scope
            scopes = parse_scopes();
            advance_on_semicolon();
        }
        expect(TokenType::kw_label, "Expected a label identifier for the first label in function body");
        body = parse_labels();
        expect(TokenType::rbrace, "Expected '}' after function body");
    }
    return std::make_shared<Function>(tok, name, attributes, params, varargs, return_type, debug_info, scopes, body);
}

GlobalItemPtr Parser::parse_global_item(){
    GlobalItemPtr item;
    if(this->curr_tok.type == TokenType::kw_fn){
        item = std::make_shared<GlobalItem>(parse_function());
        advance_on_semicolon();
    }
    else if(this->curr_tok.type == TokenType::hash){
        item = std::make_shared<GlobalItem>(parse_attribute());
        expect(TokenType::semicolon, "Expected ';' after global attribute declaration");
    }
    else if(this->curr_tok.type == TokenType::kw_let){
        item = std::make_shared<GlobalItem>(parse_instruction_stmt(true));
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