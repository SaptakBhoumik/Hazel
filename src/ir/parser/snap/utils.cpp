#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include <cstddef>
#include <iostream>

namespace LIRA {
namespace IR {
void Parser::advance(){
    this->curr_index++;
    if(this->curr_index < this->toks.size()){
        this->curr_tok = this->toks[this->curr_index];
    }
}
void Parser::advance_on_semicolon(){
    if(peek().type == TokenType::semicolon){
        this->advance();
    }
}
Token Parser::peek(std::size_t i) const{
    std::size_t index = this->curr_index + i;
    if(index < this->toks.size()){
        return this->toks[index];
    }
    Token res;
    res.type = TokenType::eof;
    return res;
}
void Parser::expect(TokenType expected_type, std::string msg,std::string submsg,std::string ecode){
    //check if the next toke is what we expect or else show error
    if(peek().type != expected_type){
        if(msg==""){
            msg="expected token of type " + to_string(expected_type) +", got " + to_string(peek().type) + " instead";
        }
        
        error(peek(),msg,submsg,ecode);
    }
    advance();
}

[[noreturn]] void Parser::error(Token tok, std::string msg,std::string submsg,std::string ecode){
    //display error
    Utils::Location loc{tok.line, tok.col, this->filename, tok.source_line};
    Utils::Diagnostic err = {loc,
                   std::string(msg),
                   submsg,
                   ecode,
        };

    display(err);
    exit(1);
}
}
}