#pragma once
#include "ir/lexer/token.hpp"
#include "ir/lexer/lexer.hpp"
#include "utils/error.hpp"
#include "utils/graph.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

namespace Hazel {
namespace Snap{
namespace IR {
class DebugInfo{
    Token tok;//the '!' token for error reporting
    Token file_name;//A string literal
    Token line;//A number literal
    Token column;//A number literal
    public:
    DebugInfo(Token tok, Token file_name, Token line, Token column);

    Token get_file_name() const;
    Token get_line() const;
    Token get_column() const;

    Token get_token() const;
    std::string to_string() const;
};
using DebugInfoPtr = std::shared_ptr<DebugInfo>;

class TypeExpr;
using TypeExprPtr = std::shared_ptr<TypeExpr>;

enum class TypeExprKind:std::uint8_t{
    NamedTypeExpr,
    IntTypeExpr,
    DecimalTypeExpr,
    StringTypeExpr,
    VoidTypeExpr,
    PtrTypeExpr,    
    ArrayTypeExpr,
    StructTypeExpr,
    FuncTypeExpr,
    LabelTypeExpr,
};
class TypeExpr{
    protected:
    Token tok;//the token for error reporting
    TypeExprKind kind;
    public:
    TypeExpr(Token tok, TypeExprKind kind);

    virtual void set_token(Token tok) final;//Change the token type in case of type reduction. NOTE:DONT USE TOKEN AS THE NAME OF ANY TYPE. OR ELSE WE WILL HAVE ISSUE WHEN WE CHANGE IT

    virtual TypeExprKind get_kind() const final;
    virtual Token get_token() const final;
    virtual std::size_t get_size() const = 0;//It is in bytes. Few bytes are extra allocated for missing field. The size is the size we have to allocate to carry it in register of our VM
    virtual std::string to_string() const = 0;

    virtual ~TypeExpr() = default;
};

using TypeExprPtr = std::shared_ptr<TypeExpr>;

class NamedTypeExpr : public TypeExpr{
    Token name;
    public:
    NamedTypeExpr(Token tok);
    
    Token get_name() const;
    
    std::size_t get_size() const override;//Return 0 cuz temporary type expr. Eleminated by type reduction. So size is unknown
    std::string to_string() const override;
};

class IntTypeExpr : public TypeExpr{
    public:
    IntTypeExpr(Token tok);

    std::size_t get_size() const override;
    std::string to_string() const override;
};

class DecimalTypeExpr : public TypeExpr{
    public:
    DecimalTypeExpr(Token tok);

    std::size_t get_size() const override;
    std::string to_string() const override;
};

class StringTypeExpr : public TypeExpr{
    public:
    StringTypeExpr(Token tok);

    std::size_t get_size() const override;
    std::string to_string() const override;
};

class VoidTypeExpr : public TypeExpr{
    public:
    VoidTypeExpr();

    std::size_t get_size() const override;
    std::string to_string() const override;
};


class PtrTypeExpr : public TypeExpr{
    //ptr<element_size>
    std::uint16_t element_size;
    public:
    PtrTypeExpr(Token tok, std::uint16_t element_size);

    std::uint16_t get_element_size() const;

    std::size_t get_size() const override;
    std::string to_string() const override;
};

class ArrayTypeExpr : public TypeExpr{
    //[type] 
    Token tok;
    TypeExprPtr base_type;
    public:
    ArrayTypeExpr(Token tok, TypeExprPtr base_type);

    TypeExprPtr get_basetype() const;
    
    std::size_t get_size() const override;
    std::string to_string() const override;
};


class StructTypeExpr : public TypeExpr{
    //{field1:type, field2:type, ... }
    Token tok;//the '{' token for error reporting
    std::vector<TypeExprPtr> fields;
    bool packed = false;//Packed means the entire struct is missing if one is missing. Default struct items are put in register but this one is packed in a tight malloc memory
    public:
    StructTypeExpr(Token tok, std::vector<TypeExprPtr> fields, bool packed);

    std::vector<TypeExprPtr> get_fields() const;
    bool is_packed() const;

    std::size_t get_size() const override;
    std::string to_string() const override;
};

class LabelTypeExpr : public TypeExpr{
    std::vector<TypeExprPtr> param_types;
    public:
    LabelTypeExpr(Token tok, std::vector<TypeExprPtr> param_types);

    std::vector<TypeExprPtr> get_param_types() const;

    std::size_t get_size() const override;
    std::string to_string() const override;
};

class FuncTypeExpr : public TypeExpr{
    //fn(type1,type2, ... ) -> return_type
    Token tok;//the 'fn' token for error reporting
    std::vector<TypeExprPtr> param_types;
    TypeExprPtr return_type;//To return multiple value, use a unpacked struct(Very effecient) or packed and lose effeciency
    public:
    FuncTypeExpr(Token tok, std::vector<TypeExprPtr> param_types, TypeExprPtr return_type);

    std::vector<TypeExprPtr> get_param_types() const;
    TypeExprPtr get_return_type() const;

    std::size_t get_size() const override;
    std::string to_string() const override;
};


enum class LiteralKind:std::uint8_t{
    ZeroInitLiteralExpr,
    PoisonLiteralExpr,
    NamedLiteralExpr, // Stuff like name,%name,$name
    NumLiteralExpr,//Can be float or int. We will figure out which one it is based on the type it is being assigned to
    StringLiteralExpr,
    ArrayLiteralExpr,
    StructLiteralExpr,
    LabelLiteralExpr,
    FuncLiteralExpr,
};

class LiteralExpr{
    protected:
    Token tok;//the token for error reporting
    TypeExprPtr type;
    LiteralKind kind;
    public:
    LiteralExpr(Token tok, LiteralKind kind, TypeExprPtr type);

    virtual void set_type(TypeExprPtr type) final;

    virtual TypeExprPtr get_type() const final;
    virtual LiteralKind get_kind() const final;
    virtual Token get_token() const final;
    virtual std::string to_string() const = 0;

    virtual ~LiteralExpr() = default;
};

using LiteralExprPtr = std::shared_ptr<LiteralExpr>;

class ZeroInitLiteralExpr : public LiteralExpr{
    public:
    ZeroInitLiteralExpr(Token tok, TypeExprPtr type);

    std::string to_string() const override;
};
class PoisonLiteralExpr : public LiteralExpr{
    public:
    PoisonLiteralExpr(Token tok, TypeExprPtr type);

    std::string to_string() const override;
};
class NamedLiteralExpr : public LiteralExpr{
    public:
    NamedLiteralExpr(Token name, TypeExprPtr type);

    Token get_name() const;

    std::string to_string() const override;
};
class NumLiteralExpr : public LiteralExpr{
    public:
    NumLiteralExpr(Token value, TypeExprPtr type);

    Token get_value() const;

    std::string to_string() const override;
};
class StringLiteralExpr : public LiteralExpr{
    public:
    StringLiteralExpr(Token value, TypeExprPtr type);

    Token get_value() const;

    std::string to_string() const override;
};
//For types like ArrayLiteral, StructLiteral, LabelLiteral, FuncLiteral, the type in the field/element is set to nullptr. After type reduction, u call set_arg_type() to set the type of all the args and all that
class ArrayLiteralExpr : public LiteralExpr{
    //[elem1, elem2, ... ]
    std::vector<LiteralExprPtr> elements;
    public:
    ArrayLiteralExpr(Token tok, std::vector<LiteralExprPtr> elements, TypeExprPtr type);

    std::vector<LiteralExprPtr> get_elements() const;

    std::string to_string() const override;
};
class StructLiteralExpr : public LiteralExpr{
    //{value1, value2, ... }
    Token tok;//the '{' token for error reporting
    std::vector<LiteralExprPtr> fields;
    bool packed = false;//Packed means the entire struct is missing if one is missing. Default struct items are put in register but this one is packed in a tight malloc memory
    public:
    StructLiteralExpr(Token tok, std::vector<LiteralExprPtr> fields, bool packed, TypeExprPtr type);

    std::vector<LiteralExprPtr> get_fields() const;
    bool is_packed() const;

    std::string to_string() const override;
};
class LabelLiteralExpr : public LiteralExpr{
    //label(elem1, elem2, ... )
    std::vector<LiteralExprPtr> args;
    public:
    LabelLiteralExpr(Token tok, std::vector<LiteralExprPtr> args, TypeExprPtr type);

    Token get_name() const;
    std::vector<LiteralExprPtr> get_args() const;

    std::string to_string() const override;
};
class FuncLiteralExpr : public LiteralExpr{
    //func(elem1, elem2, ... )//Can be external function also
    std::vector<LiteralExprPtr> args;
    public:
    FuncLiteralExpr(Token tok, std::vector<LiteralExprPtr> args, TypeExprPtr type);

    Token get_name() const;
    std::vector<LiteralExprPtr> get_args() const;

    std::string to_string() const override;
};

class InstructionStmt{
    private:
    Token tok;//the 'let'/instruction token for error reporting
    Token instruction;//the instruction token
    std::optional<std::pair<Token, TypeExprPtr>> name;//Empty if we dont assign the statement to a variable
    std::vector<LiteralExprPtr> params;
    DebugInfoPtr debug_info = nullptr;
    public:
    InstructionStmt(Token tok, Token instruction, std::optional<std::pair<Token, TypeExprPtr>> name, std::vector<LiteralExprPtr> params, DebugInfoPtr debug_info);

    Token get_instruction() const;
    std::optional<std::pair<Token, TypeExprPtr>> get_name() const;
    std::vector<LiteralExprPtr> get_params() const;
    DebugInfoPtr get_debug_info() const;
    Token get_token() const;
    std::string to_string() const;
};  

using InstructionStmtPtr = std::shared_ptr<InstructionStmt>;

class Label {
    /*
    @loop{
        ...
    }
    */
    Token tok;//The 'label' token for error reporting
    Token name;//The ``@name`` token for error reporting and identifying the label. Note that the value of this token includes the @ prefix (e.g. @loop)
    std::vector<std::pair<Token, TypeExprPtr>> params;
    std::unordered_map<std::string,std::uint64_t> parameter_map;//For faster access. The second argument is the index of the parameter. Calculated automatically
    std::vector<InstructionStmtPtr> statements;
    std::unordered_map<std::string,std::uint64_t> local_var_map;//For faster access. The second argument is the index of the local variable in the instructions. Calculated automatically
    DebugInfoPtr debug_info = nullptr;
    public:
    Label(Token tok, Token name, std::vector<InstructionStmtPtr> statements, std::vector<std::pair<Token, TypeExprPtr>> params, DebugInfoPtr debug_info, bool calculate_maps);//We dont calulate the map by default. We do it only after type reduction and after verifying there is no duplicate variable or unknown variable

    Token get_name() const;
    Token get_token() const;
    std::vector<InstructionStmtPtr> get_statements() const;
    std::vector<std::pair<Token, TypeExprPtr>> get_params() const;
    DebugInfoPtr get_debug_info() const;
    std::string to_string() const;

    std::optional<std::pair<Token, TypeExprPtr>> get_parameter(std::string name) const;//The first element is the token of parameter
    std::optional<std::pair<Token, InstructionStmtPtr>> get_local_var_inst(std::string name) const;//Instruction where it was defined. The first element is the token of variable
    std::optional<std::pair<Token, TypeExprPtr>> get_local_var(std::string name) const;//The first element is the token of variable
    std::optional<Utils::triplet<Token, TypeExprPtr, bool>> get_local_var_or_param(std::string name) const;//The first element is the token of variable/parameter, second is the type of variable/parameter, third is true if it is a parameter and false if it is a local variable
};

using LabelPtr = std::shared_ptr<Label>;

class Function {
    Token tok;//The ``fn`` token for error reporting
    Token name;//The function name token for error reporting and identifying the function
    std::vector<std::pair<Token, TypeExprPtr>> params;
    TypeExprPtr return_type;
    std::unordered_map<std::string,std::uint64_t> parameter_map;//For faster access. The second argument is the index of the parameter. Calculated automatically
    std::vector<LabelPtr> body;//Empty if it is just a function declaration without a body. A function declaration with body must have atleast one label
                               //The first label is the entry point. Atleast one label is needed so we for forward declaration without body, this vector has 0 length
    std::unordered_map<std::string,std::uint64_t> label_map;//For faster access. The second argument is the index of the label in the body. Calculated automatically
    std::unordered_map<std::string,std::uint64_t> local_var_map;//For faster access. The second argument is the index of the local variable in the labels
    DebugInfoPtr debug_info = nullptr;
    public:
    Function(Token tok, Token name, std::vector<std::pair<Token, TypeExprPtr>> params, TypeExprPtr return_type,
             std::vector<LabelPtr> body, DebugInfoPtr debug_info, bool calculate_maps);//We dont calulate the map by default. We do it only after type reduction and after verifying there is no duplicate variable or unknown variable

    Token get_name() const;
    Token get_token() const;
    std::vector<std::pair<Token, TypeExprPtr>> get_params() const;
    TypeExprPtr get_return_type() const;
    bool is_signature_only() const;//Returns true if it is just a function declaration without a body.
    std::vector<LabelPtr> get_body() const;
    DebugInfoPtr get_debug_info() const;
    std::string to_string() const;

    std::optional<std::pair<Token, TypeExprPtr>> get_parameter(std::string name) const;
    std::optional<std::pair<Token, LabelPtr>> get_label(std::string name) const;
    std::optional<std::pair<Token, LabelPtr>> get_label_of_local_var(std::string name) const;
    std::optional<Utils::triplet<Token, TypeExprPtr, bool>> get_local_var_or_param(std::string name) const;//The first element is the token of variable/parameter, second is the type of variable/parameter, third is true if it is a parameter and false if it is a local variable
};

using FunctionPtr = std::shared_ptr<Function>;

class Program{
    std::vector<FunctionPtr> items;
    std::unordered_map<std::string,std::uint64_t> function_map;//For faster access. The second argument is the index of the function in the items. Calculated automatically
    std::optional<Utils::Graph> call_graph;
    public:
    Program(std::vector<FunctionPtr> items, std::optional<Utils::Graph> call_graph, bool calculate_map);//We dont calulate the map by default. We do it only after type reduction and after verifying there is no duplicate variable or unknown variable

    std::vector<FunctionPtr> get_items() const;
    std::optional<std::pair<Token, FunctionPtr>> get_function(std::string name) const;
    std::string to_string() const;
};

using ProgramPtr = std::shared_ptr<Program>;

namespace TypeUtils{
bool is_type_equal(TypeExprPtr type1, TypeExprPtr type2);//Expects reduced types
}
}
}
}