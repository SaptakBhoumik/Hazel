#include "ir/lexer/lexer.hpp"
#include "ir/lexer/token.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

namespace Hazel {
namespace Snap{
namespace IR {
Lexer::Lexer(const std::string& source, const std::string& filename): input(source), filename(filename){
    if(this->input.empty()){
        this->result.push_back(Token{0, "", "<eof>", 0, 0, 1, TokenType::eof});
        return;
    }
    this->source_lines = Lexer::split_lines(this->input);
    this->curr_char = this->input[0];
    this->curr_line = this->source_lines[0];
    this->lex();
    this->finalize();
}
std::string Lexer::get_filename() const{
    return this->filename;
}

TokenList Lexer::get_tokens() const{
    return this->result;
}

void Lexer::push(const std::string& value, TokenType type, size_t start, size_t end){
    this->result.emplace_back(Token{
        this->col,
        this->curr_line,
        value,
        start,
        end,
        this->line_num,
        type
    });
}

void Lexer::push_current(TokenType type){
    this->push(
        std::string(1, this->curr_char),
        type,
        this->curr_index,
        this->curr_index + 1
    );
}

void Lexer::flush_keyword(){
    if(this->curr_keyword.empty()){
        return;
    }

    const std::string& kw = this->curr_keyword;
    const size_t start = this->curr_index - kw.size();
    const size_t end = this->curr_index;

    // raw string prefix: the 'r' immediately before a quote
    if(kw == "r" && (this->curr_char == '"' || this->curr_char == '\'')){
        this->push(kw, TokenType::raw, start, end);
        this->curr_keyword.clear();
        return;
    }
    //No. Spaceing is not allowed after f or r if u want raw or formated string

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"fn", TokenType::kw_fn},
        {"let", TokenType::kw_let},
        {"label", TokenType::kw_label},
        {"null", TokenType::kw_null},
        {"zeroinitializer", TokenType::kw_zeroinitializer},
        {"poison", TokenType::kw_poison},
        {"integer", TokenType::kw_integer},
        {"decimal", TokenType::kw_decimal},
        {"string", TokenType::kw_string},
        {"ptr", TokenType::kw_ptr},
        {"True", TokenType::kw_true},
        {"False", TokenType::kw_false}
    };

    TokenType type;
    auto it = keywords.find(kw);
    if(it != keywords.end()){
        type = it->second;
    } 
    else if(Lexer::is_integer(kw)){
        type = TokenType::integer;
    } 
    else if(Lexer::is_decimal(kw)){
        type = TokenType::decimal;
    }
    else{
        auto _type = get_identifier_type(kw);
        if(!_type.has_value()){
            type = TokenType::global_func_identifier; // default to global function identifier if no prefix is found
            this->report_error("Unknown identifier type: " + kw, "Identifiers must start with $, %, @, ^, or # to indicate their type.");
        }
        else{
            type = _type.value();
        }
    }

    this->push(kw, type, start, end);
    this->curr_keyword.clear();
}


void Lexer::handle_newline_tracking(){
    this->line_num++;
    this->col = 0;
    if(this->line_num <= this->source_lines.size()){
        this->curr_line = this->source_lines[this->line_num - 1];
    }
}


void Lexer::lex(){
    while (true){
        switch (this->curr_char){
            // ---- string literals ----
            case '/':{
                this->lex_slash();
                break;
            }
            case '"':
            case '\'': {
                this->flush_keyword();
                this->lex_string();
                break;
            }

            // ---- parentheses ----
            case '(': {
                this->flush_keyword();
                this->paren_depth++;
                this->push_current(TokenType::lparen);
                break;
            }
            case ')': {
                this->flush_keyword();
                if(this->paren_depth == 0){
                    this->report_error("')' without matching '('");
                } 
                else{
                    this->paren_depth--;
                }
                this->push_current(TokenType::rparen);
                break;
            }

            // ---- square brackets ----
            case '[': {
                this->flush_keyword();
                this->bracket_depth++;
                this->push_current(TokenType::lbracket);
                break;
            }
            case ']': {
                this->flush_keyword();
                if(this->bracket_depth == 0){
                    this->report_error("']' without matching '['");
                } 
                else{
                    this->bracket_depth--;
                }
                this->push_current(TokenType::rbracket);
                break;
            }

            // ---- curly braces ----
            case '{': {
                this->flush_keyword();
                this->brace_depth++;
                this->push_current(TokenType::lbrace);
                break;
            }
            case '}': {
                this->flush_keyword();
                if(this->brace_depth == 0){
                    this->report_error("'}' without matching '{'");
                } 
                else{
                    this->brace_depth--;
                }
                this->push_current(TokenType::rbrace);
                break;
            }

            case '<': {
                this->flush_keyword();
                this->angel_depth++;
                this->push_current(TokenType::langel);
                break;
            }
            case '>': {
                this->flush_keyword();
                if(this->angel_depth == 0){
                    this->report_error("'>' without matching '<'");
                } 
                else{
                    this->angel_depth--;
                }
                this->push_current(TokenType::rangel);
                break;
            }
            
            //Others
            case '=' : {
                this->flush_keyword();
                this->push_current(TokenType::assign);
                break;
            }
            case '!' : {
                this->flush_keyword();
                this->push_current(TokenType::bang);
                break;
            }
            case '-' : {
                if(peek() == '>'){
                    this->flush_keyword();
                    this->advance();
                    this->push("->", TokenType::arrow, this->curr_index - 1, this->curr_index + 1);
                    break;
                }
                else{
                    this->curr_keyword += this->curr_char;
                    break;
                }
            }
            case ',' : {
                this->flush_keyword();
                this->push_current(TokenType::comma);
                break;
            }
            case ':' : {
                this->flush_keyword();
                this->push_current(TokenType::colon);
                break;
            }      

            // ---- whitespace (Hazel is not indentation-sensitive) ----
            case ' ':
            case '\t': {
                this->flush_keyword();
                break;
            }

            // ---- newlines/semicolons ----
            case '\n': {
                this->flush_keyword();
                this->handle_newline_tracking();
                break;
            }
            case '\r': {
                this->flush_keyword();
                if(this->peek() == '\n'){
                    this->advance();  // consume the \n of \r\n
                }
                this->handle_newline_tracking();
                break;
            }
            case ';': {
                this->flush_keyword();
                if(!this->result.empty()){
                    if(this->result.back().type == TokenType::semicolon){
                        this->result.pop_back();//We want the last semicolon to be the one we just encountered, not some random one from before. 
                    }
                }   
                this->push_current(TokenType::semicolon);
                break;
            }

            // ---- accumulate keyword / identifier / number ----
            default: {
                this->curr_keyword += this->curr_char;
                break;
            }
        }

        if(!this->advance()){
            // this->flush_keyword();//Yeah no need to flush keyword because finalize will do it and we are at end of file so no more tokens anyway.
            break;
        }
    }
}

void Lexer::finalize(){
    this->flush_keyword();
    if(this->paren_depth != 0){
        this->report_error("Unclosed '('", "expected a matching ')'");
    }
    if(this->bracket_depth != 0){
        this->report_error("Unclosed '['", "expected a matching ']'");
    }
    if(this->brace_depth != 0){
        this->report_error("Unclosed '{'", "expected a matching '}'");
    }
    if(this->angel_depth != 0){
        this->report_error("Unclosed '<'", "expected a matching '>'");
    }
    // Let display_all handle printing + exit(1) if there are errors.
    display_all(this->diagnostics);

    // if(!this->result.empty()){
    //     if(this->result.back().type != TokenType::semicolon){
    //         this->push(";", TokenType::semicolon,this->curr_index, this->curr_index + 1);
    //     }
    // }

    this->push("<eof>", TokenType::eof,this->curr_index, this->curr_index + 1);
}


void Lexer::lex_string(){
    const char quote = this->curr_char;
    const size_t start = this->curr_index + 1;
    std::string str;

    if(!this->advance()){
        this->report_error("Unexpected end of file: unterminated string literal");
        return;
    }

    while (this->curr_char != quote){
        if(this->curr_char == '\\'){
            // Escape sequence: unconditionally consume the next character.
            // This correctly handles \\, \", \n, \t, etc. without any
            // back-tracking or goto tricks.
            str += this->curr_char;                          // '\'
            if(!this->advance()){
                this->report_error("Unexpected end of file inside string escape sequence");
                return;
            }
            str += this->curr_char;                          // escaped char
        } 
        else if(this->curr_char == '\n'){
            str += this->curr_char;
            this->handle_newline_tracking();
        } 
        else if(this->curr_char == '\r'){
            str += this->curr_char;
            if(this->peek() == '\n'){
                this->advance();
                str += this->curr_char;
            }
            this->handle_newline_tracking();
        } 
        else{
            str += this->curr_char;
        }

        if(!this->advance()){
            this->report_error("Unexpected end of file: unterminated string literal");
            return;
        }
    }
    if(this->result.back().type == TokenType::raw){
        result.pop_back();
        this->push(str, TokenType::raw_string, start-1, this->curr_index);
    }
    else{
        this->push(str, TokenType::string, start, this->curr_index);
    }
}

void Lexer::lex_slash(){
    if(this->peek() == '/'){
        this->flush_keyword();
        // Line comment: consume everything up to (but not including) the newline.
        // The newline itself will be processed normally on the next iteration.
        this->advance();  // consume second '/'
        while (this->peek() != '\n' && this->peek() != '\r' && this->peek() != '\0'){
            if(!this->advance()){
                return;
            }
        }
        return;  // no token emitted
    }

    else if(this->peek() == '*'){
        this->flush_keyword();
        // Block comment
        this->advance();  // consume '*'
        while (true){
            if(!this->advance()){
                this->report_error("Unexpected end of file: unterminated block comment");
                return;
            }
            if(this->curr_char == '\n'){
                this->handle_newline_tracking();
            } 
            else if(this->curr_char == '\r'){
                if(this->peek() == '\n'){ 
                    this->advance();
                }
                this->handle_newline_tracking();
            } 
            else if(this->curr_char == '*' && this->peek() == '/'){
                this->advance();  // consume '/'
                return;
            }
        }
    }
    else{
        this->curr_keyword += this->curr_char; 
    }
}
}
} 
} 