#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"
#include "utils/graph.hpp"
#include <map>

namespace Hazel {
namespace Snap{
namespace IR {
class Parser {
    std::size_t curr_index = 0;
    Token curr_tok;
    std::vector<Token> toks;
    std::vector<std::pair<Token,TypeExprPtr>> global_typedef;//This thing is not stored in the ProgramPtr itself because this is more of an intermediate and wont be needed after reduction
    ProgramPtr ast;
    std::string filename;

    // Utils
    void advance();
    void advance_on_semicolon();
    Token peek(std::size_t i=1) const;// peek the token at curr_index + i without advancing
    void expect(TokenType expected_type, std::string msg="",std::string submsg="",std::string ecode="");
    [[noreturn]] void error(Token tok, std::string msg,std::string submsg="",std::string ecode="") const;

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
    InstructionStmtPtr parse_instruction_stmt();//On 'let' token or instruction token
    LabelPtr parse_label();//On label identifier token
    std::vector<LabelPtr> parse_labels();
    FunctionPtr parse_function();//On 'fn' token
    FunctionPtr parse_global_item();//If fn then parses it else it is a global type def that is parsed and stored in global_typedef

    void parse();


    void basic_typecheck();//Not the full thing. IT doesnt check if the types are proper for the instruction. It does a basic symbol table and that is all. Need seperate check for that
    bool self_reference_type(TypeExprPtr type, std::string name) const;
    std::unordered_map<std::string, TypeExprPtr> construct_type_table() const;//Constructs a type table from the global typedefs. The types in the type table are reduced types
    //The types in type_table are expected to be reduced types.
    TypeExprPtr reduce_type_expr(TypeExprPtr type, std::unordered_map<std::string, TypeExprPtr> type_table) const;//Reduces the type expression by replacing all the named types with their actual types. Throws error if a named type is not found in the type table
    LiteralExprPtr reduce_literal_expr(LiteralExprPtr literal, Utils::Graph& call_graph, std::string current_func, std::unordered_map<std::string, TypeExprPtr> type_table, 
                                       std::unordered_map<std::string, TypeExprPtr> func_symbol_table, std::unordered_map<std::string, TypeExprPtr> label_symbol_table,
                                       std::unordered_map<std::string, TypeExprPtr> local_var_table, std::unordered_map<std::string, TypeExprPtr> label_param_table) const;
                                       //Label parameter may shadow the previous local variable so check type with label_param_table first before local_var_table.
    FunctionPtr reduce_function(FunctionPtr func, Utils::Graph& call_graph, std::vector<std::pair<Token, TypeExprPtr>> reduced_params, std::unordered_map<std::string, TypeExprPtr> type_table, std::unordered_map<std::string, TypeExprPtr> func_symbol_table) const;

    public:
    Parser(const std::vector<Token>& toks, const std::string& filename);
    ProgramPtr get_ast();
};
}
}
}