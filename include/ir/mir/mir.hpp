#pragma once
#include "ir/ast/ast.hpp"
#include "ir/lexer/token.hpp"

namespace Hazel {
namespace Snap{
namespace IR {
enum class MirLiteralKind: std::uint64_t{
    PoisonLiteral,
    NamedLiteral,
    NumLiteral,
    StringLiteral,
    ArrayLiteral,
    StructLiteral,
    LabelLiteral,
    FuncLiteral
};

class MirLiteralExpr{
    protected:
    MirLiteralKind kind;
    LiteralExprPtr literal_expr;
    public:
    MirLiteralExpr(LiteralExprPtr literal_expr, MirLiteralKind kind);
    virtual ~MirLiteralExpr() = default;

    virtual MirLiteralKind get_kind() const final;
    virtual IR::LiteralExprPtr get_literal_expr() const final;
    virtual IR::TypeExprPtr get_type() const final;
    virtual bool is_immediate() const = 0;//If it is a literal that is directly used in the instruction. 
    virtual std::string to_string() const = 0;
};

using MirLiteralExprPtr = std::shared_ptr<MirLiteralExpr>;

enum class MirInstType: std::uint64_t{
    AddInst,
    SubInst,
    MulInst,
    DivInst,
    ModInst,
    MinInst,
    MaxInst,

    WeightedAddInst,

    AndInst,
    OrInst,
    XorInst,

    EqInst,
    NeInst,
    GtInst,
    GeInst,
    InRangeInst,
    NotInRangeInst,

    AbsInst,
    CeilInst,
    FloorInst,
    IntegralPartInst,
    FractionalPartInst,
    RoundNearestInst,
    RoundEvenInst,
    SqrtInst,
    HasValueInst,
    HasNoValueInst,

    BrEqInst,
    BrGtInst,
    BrOrInst,
    BrAndInst,
    BrXorInst,
    BrInRangeInst,
    BrInst,

    NopInst,
    HaltInst,
    TrapInst,
    TrapIfInst,
    TrapIfMissingInst,
    TraceInst,
    TraceIfInst,
    TraceIfMissingInst,
    PrintInst,
};
class MirInstructionStmt{
    protected:
    MirInstType inst_type;
    std::optional<std::pair<std::string, TypeExprPtr>> dest;
    InstructionStmtPtr instruction_stmt;
    bool pure;
    bool const_;
    bool compiler_intrinsic;
    public:
    MirInstructionStmt(InstructionStmtPtr instruction_stmt, std::optional<std::pair<std::string, TypeExprPtr>> dest, MirInstType inst_type, bool pure, bool const_, bool compiler_intrinsic);
    virtual ~MirInstructionStmt() = default;

    virtual std::optional<std::pair<std::string, TypeExprPtr>> get_destination() const final;
    virtual MirInstType get_inst_type() const final;
    virtual IR::InstructionStmtPtr get_instruction_stmt() const final;//For error reporting
    virtual IR::DebugInfoPtr get_debug_info() const final;
    virtual bool is_pure() const final;//Diffrent output for same input can happen but it doesnt change the state of the program. So can be removed if the output is not used
    virtual bool is_const() const final;//Same input = same output. May change the state of the program. Pair it with pure to get the traditional defination of const instruction
    virtual bool is_compiler_intrinsic() const final;//This is a compiler intrinsic instruction. It is safe to remove but the output compiled program may be diffrent. So we dont remove them
    virtual std::string to_string() const = 0;
};

using MirInstructionStmtPtr = std::shared_ptr<MirInstructionStmt>;
}
}
}