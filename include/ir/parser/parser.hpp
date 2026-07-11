#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include <map>

namespace Hazel {
namespace Snap{
namespace IR {
class Parser {
    std::size_t curr_index = 0;
    Token curr_tok;
    std::vector<Token> toks;
    std::vector<std::pair<Token,TypeExprPtr>> global_typedef;//This thing is not stored in the ProgramPtr itself because this is more of an intermediate and wont be needed after reduction

    std::string filename;

    // Utils
    void advance();
    void advance_on_semicolon();
    Token peek(std::size_t i=1) const;// peek the token at curr_index + i without advancing
    void expect(TokenType expected_type, std::string msg="",std::string submsg="",std::string ecode="");
    [[noreturn]] void error(Token tok, std::string msg,std::string submsg="",std::string ecode="");

    // Parse type expressions
    TypeExprPtr parse_type_expr();
    TypeExprPtr parse_ptr_type_expr();
    TypeExprPtr parse_array_type_expr();
    TypeExprPtr parse_packed_struct_type_expr();
    TypeExprPtr parse_struct_type_expr();
    TypeExprPtr parse_func_type_expr();
    TypeExprPtr parse_label_type_expr();

    // Parse literal expressions
    LiteralExprPtr parse_literal_expr(bool parse_type);//If true then we parse the type of the literal as well
    LiteralExprPtr parse_array_literal_expr(TypeExprPtr type);
    LiteralExprPtr parse_packed_struct_literal_expr(TypeExprPtr type);
    LiteralExprPtr parse_struct_literal_expr(TypeExprPtr type);
    LiteralExprPtr parse_label_literal_expr(TypeExprPtr type);
    LiteralExprPtr parse_func_literal_expr(TypeExprPtr type);


    // Parse attributes
    DebugInfoPtr parse_debug_info();//On ! token. 
    TypeExprPtr parse_type_expr_pair();//type:expr pair for args
    InstructionStmtPtr parse_instruction_stmt(bool is_global);//On 'let' token or instruction token
    LabelPtr parse_label();//On label identifier token
    std::vector<LabelPtr> parse_labels();
    FunctionPtr parse_function();//On 'fn' token
    FunctionPtr parse_global_item();//If fn then parses it else it is a global type def that is parsed and stored in global_typedef
    public:
    Parser(const std::vector<Token>& toks, const std::string& filename);
    ProgramPtr parse();
    std::vector<std::pair<Token,TypeExprPtr>> get_global_typedef() const;
};
}
}
}