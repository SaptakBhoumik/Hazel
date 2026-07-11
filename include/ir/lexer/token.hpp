#pragma once
#include <cstddef>
#include <string>
#include "utils/utils.hpp"
namespace Hazel {
namespace Snap{
namespace IR {
enum class TokenType {
    eof,

    // Literals
    integer,
    decimal,
    string,
    raw, // the 'r' prefix before a raw string literal, used as an intermediate token to signal that the following string literal is raw. Parser dont see it
    raw_string,

    global_func_identifier, // $name
    local_identifier,           // %name
    label_identifier,           // @name
    instruction_identifier,     // .name
    extern_func_identifier,     // ^name
    type_identifier,            // #name

    // Brackets
    lparen,           // (
    rparen,           // )
    lbracket,         // [
    rbracket,         // ]
    lbrace,           // {
    rbrace,           // }
    langel,           // <
    rangel,           // >

    // Others
    bang,             // !  (debug info)
    assign,           // =
    arrow,            // -> (function return type annotation)
    comma,            // ,
    colon,            // :

    //Keywords
    //Note if `name` is a keyword then also u can use it as a variable name by prefixing it with $ or % or @ or . (e.g. $fn, %fn, @fn, .fn are all valid identifiers even though fn is a keyword)
    kw_fn,              // 'fn' keyword (for fn definitions)
    kw_let,             // 'let' keyword (for let declarations)
    kw_label,           // 'label' keyword (for label definitions)
    kw_zeroinit, // 'zeroinit' keyword (for zero initializer constant)
    kw_poison,          // 'poison' keyword (for poison value constant). Means missing value. 
    kw_integer,         // 'integer' keyword (for int type)
    kw_decimal,         // 'decimal' keyword (for decimal type)
    kw_string,          // 'string' keyword (for string type)
    kw_ptr,             // 'ptr' keyword (for pointer type)

    semicolon,
};

std::string to_string(TokenType type);

struct Token {
    size_t col;          // column of the token start
    std::string source_line;  // the full source line for diagnostics
    std::string value;        // raw text of the token
    //start and end are probably useless but keeping it just in case. They are byte offsets from the start of the file.
    size_t start;        // byte offset start
    size_t end;          // byte offset end  
    size_t line;         // 1-based line number
    TokenType type;
};
std::string to_string(const Token& tok);

std::ostream& operator<<(std::ostream& os, const Token& tok);

bool is_identifier(TokenType type);

struct TokenValueEqual {
    inline bool operator()(const Token& a, const Token& b) const{
        return a.value == b.value;
    }
};

// Hash
struct TokenValueHash {
    inline std::size_t operator()(const Token& obj) const{
        return std::hash<std::string>{}(obj.value);
    }
};
}
} 
} 