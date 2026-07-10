#pragma once
#include "token.hpp"
#include "utils/error.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace Hazel {
namespace Snap{
namespace IR {
using TokenList = std::vector<Token>;
class Lexer {
    // ---- stored data ----
    TokenList result;
    std::string input;
    std::string filename;
    std::vector<std::string> source_lines;
    std::vector<Utils::Diagnostic> diagnostics;   // collected errors/warnings

    // ---- cursor state ----
    size_t curr_index = 0;
    char curr_char = '\0';
    std::string curr_keyword;
    std::string curr_line;
    size_t line_num = 1;
    size_t col = 0;

    // ---- bracket nesting ----
    size_t paren_depth = 0;
    size_t bracket_depth = 0;
    size_t brace_depth = 0;
    size_t angel_depth = 0;

    // ---- core loop ----
    void lex();
    void finalize();

    // ---- navigation ----
    bool advance();
    char peek(std::size_t i=1) const;// peek the char at curr_index + i without advancing

    // ---- helpers ----
    static std::vector<std::string> split_lines(const std::string& code);
    static bool is_integer(const std::string& s);
    static bool is_decimal(const std::string& s);
    static std::optional<TokenType> get_identifier_type(const std::string& s);

    // Build a Location from the current cursor position
    Utils::Location here() const;

    void flush_keyword();
    void push(const std::string& value, TokenType type, size_t start, size_t end);
    void push_current(TokenType type);
    void handle_newline_tracking();

    // Record a diagnostic (does not stop lexing immediately)
    void report(Utils::DiagLevel level,const std::string& message,const std::string& sub_message = "",const std::string& code = "");

    void report_error(const std::string& msg, const std::string& sub = "");
    void report_warning(const std::string& msg, const std::string& sub = "");


    // ---- sub-lexers ----
    void lex_string();
    void lex_slash();

public:
    Lexer(const std::string& source, const std::string& filename);
    std::string get_filename() const;
    TokenList get_tokens()  const;
    bool has_errors()  const;

    // Expose collected diagnostics so other compiler stages can merge them
    const std::vector<Utils::Diagnostic>& get_diagnostics() const;
};
}
}
} 