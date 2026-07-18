#pragma once
#include "mir.hpp"
namespace Hazel {
namespace Snap{
namespace IR {
//TODO:Mention the allowed type of register and immdiate for each instruction
//The canonical form is meant to match close to the instruction set of VM. 
class MirAddInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register + register
    //2. register + immediate
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirAddInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirSubInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register - register
    //2. immediate - register
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirSubInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};


class MirMulInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register * register
    //2. register * immediate
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirMulInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirDivInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register / register
    //2. register / immediate
    //3. immediate / register
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirDivInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirModInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register % register
    //2. register % immediate
    //3. immediate % register
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirModInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirMinInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. min(register, register)
    //2. min(register, immediate)
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirMinInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirMaxInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. max(register, register)
    //2. max(register, immediate)
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirMaxInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirWeightedAddInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register * register + register * register + ... + register
    //2. register * register + register * register + ... + immediate
    //3. immediate * register + immediate * register + ... + register
    //4. immediate * register + immediate * register + ... + immediate
    std::vector<std::pair<MirLiteralExprPtr, MirLiteralExprPtr>> weighted_params;
    MirLiteralExprPtr acc;
    public:
    MirWeightedAddInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, 
                       std::vector<std::pair<MirLiteralExprPtr, MirLiteralExprPtr>> weighted_params, MirLiteralExprPtr acc);

    std::vector<std::pair<MirLiteralExprPtr, MirLiteralExprPtr>> get_weighted_params() const;
    MirLiteralExprPtr get_acc() const;

    std::string to_string() const;
};

class MirAndInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register & register
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirAndInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirOrInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register | register
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirOrInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirXorInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register ^ register
    //2. register ^ immediate
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirXorInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};  

class MirEqInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register == register
    //2. register == immediate
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirEqInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirNeInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register != register
    //2. register != immediate
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirNeInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirGtInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register > register
    //2. register > immediate
    //3. immediate > register
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirGtInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirGeInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register >= register
    //2. register >= immediate
    //3. immediate >= register
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;
    public:
    MirGeInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    std::string to_string() const;
};

class MirInRangeInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register in range(start = register,end = register,step = register)
    //2. register in range(start = register,end = register,step = immediate)
    MirLiteralExprPtr value;
    MirLiteralExprPtr lower;
    MirLiteralExprPtr upper;
    MirLiteralExprPtr step;
    public:
    MirInRangeInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest,
                   MirLiteralExprPtr value, MirLiteralExprPtr lower, MirLiteralExprPtr upper, MirLiteralExprPtr step);
    
    MirLiteralExprPtr get_value() const;
    MirLiteralExprPtr get_lower() const;
    MirLiteralExprPtr get_upper() const;
    MirLiteralExprPtr get_step() const;

    std::string to_string() const;
};

class MirNotInRangeInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. register not in range(start = register,end = register,step = register)
    //2. register not in range(start = register,end = register,step = immediate)
    MirLiteralExprPtr value;
    MirLiteralExprPtr lower;
    MirLiteralExprPtr upper;
    MirLiteralExprPtr step;
    public:
    MirNotInRangeInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest,
                      MirLiteralExprPtr value, MirLiteralExprPtr lower, MirLiteralExprPtr upper, MirLiteralExprPtr step);
    
    MirLiteralExprPtr get_value() const;
    MirLiteralExprPtr get_lower() const;
    MirLiteralExprPtr get_upper() const;
    MirLiteralExprPtr get_step() const;

    std::string to_string() const;
};

class MirAbsInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. abs(register)
    MirLiteralExprPtr value;
    public:
    MirAbsInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirCeilInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. ceil(register)
    MirLiteralExprPtr value;
    public:
    MirCeilInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirFloorInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. floor(register)
    MirLiteralExprPtr value;
    public:
    MirFloorInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirIntegralPartInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. integral_part(register)
    MirLiteralExprPtr value;
    public:
    MirIntegralPartInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirFractionalPartInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. fractional_part(register)
    MirLiteralExprPtr value;
    public:
    MirFractionalPartInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirRoundNearestInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. round_nearest(register)
    MirLiteralExprPtr value;
    public:
    MirRoundNearestInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirRoundEvenInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. round_even(register)
    MirLiteralExprPtr value;
    public:
    MirRoundEvenInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirSqrtInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. sqrt(register)
    MirLiteralExprPtr value;
    public:
    MirSqrtInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirHasValueInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. has_value(register)
    MirLiteralExprPtr value;
    public:
    MirHasValueInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirHasNoValueInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. has_no_value(register)
    MirLiteralExprPtr value;
    public:
    MirHasNoValueInst(InstructionStmtPtr instruction_stmt, std::pair<std::string, TypeExprPtr> dest, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;
};

class MirBrEqInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. br_eq(register, register, true_label, false_label, missing_label)
    //2. br_eq(register, immediate, true_label, false_label, missing_label)
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;

    MirLiteralExprPtr true_label;
    MirLiteralExprPtr false_label;
    MirLiteralExprPtr missing_label;
    public:
    MirBrEqInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs,
                MirLiteralExprPtr true_label, MirLiteralExprPtr false_label, MirLiteralExprPtr missing_label);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    MirLiteralExprPtr get_true_label() const;
    MirLiteralExprPtr get_false_label() const;
    MirLiteralExprPtr get_missing_label() const;

    std::string to_string() const;  
};

class MirBrGtInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. br_gt(register, register, true_label, false_label, missing_label)
    //2. br_gt(register, immediate, true_label, false_label, missing_label)
    //3. br_gt(immediate, register, true_label, false_label, missing_label)
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;

    MirLiteralExprPtr true_label;
    MirLiteralExprPtr false_label;
    MirLiteralExprPtr missing_label;
    public:
    MirBrGtInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs,
                MirLiteralExprPtr true_label, MirLiteralExprPtr false_label, MirLiteralExprPtr missing_label);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    MirLiteralExprPtr get_true_label() const;
    MirLiteralExprPtr get_false_label() const;
    MirLiteralExprPtr get_missing_label() const;

    std::string to_string() const;  
};

class MirBrOrInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. br_or(register, register, true_label, false_label, missing_label)
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;

    MirLiteralExprPtr true_label;
    MirLiteralExprPtr false_label;
    MirLiteralExprPtr missing_label;
    public:
    MirBrOrInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs,
                MirLiteralExprPtr true_label, MirLiteralExprPtr false_label, MirLiteralExprPtr missing_label);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    MirLiteralExprPtr get_true_label() const;
    MirLiteralExprPtr get_false_label() const;
    MirLiteralExprPtr get_missing_label() const;

    std::string to_string() const;  
};

class MirBrAndInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. br_and(register, register, true_label, false_label, missing_label)
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;

    MirLiteralExprPtr true_label;
    MirLiteralExprPtr false_label;
    MirLiteralExprPtr missing_label;
    public:
    MirBrAndInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs,
                MirLiteralExprPtr true_label, MirLiteralExprPtr false_label, MirLiteralExprPtr missing_label);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    MirLiteralExprPtr get_true_label() const;
    MirLiteralExprPtr get_false_label() const;
    MirLiteralExprPtr get_missing_label() const;

    std::string to_string() const;  
};

class MirBrXorInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. br_xor(register, register, true_label, false_label, missing_label)
    MirLiteralExprPtr lhs;
    MirLiteralExprPtr rhs;

    MirLiteralExprPtr true_label;
    MirLiteralExprPtr false_label;
    MirLiteralExprPtr missing_label;
    public:
    MirBrXorInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr lhs, MirLiteralExprPtr rhs,
                MirLiteralExprPtr true_label, MirLiteralExprPtr false_label, MirLiteralExprPtr missing_label);
    
    MirLiteralExprPtr get_lhs() const;
    MirLiteralExprPtr get_rhs() const;

    MirLiteralExprPtr get_true_label() const;
    MirLiteralExprPtr get_false_label() const;
    MirLiteralExprPtr get_missing_label() const;

    std::string to_string() const;  
};

class MirBrInRangeInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. br_in_range(register, start = register,end = register,step = register, true_label, false_label, missing_label)
    //2. br_in_range(register, start = register,end = register,step = immediate, true_label, false_label, missing_label)
    MirLiteralExprPtr value;
    MirLiteralExprPtr lower;
    MirLiteralExprPtr upper;
    MirLiteralExprPtr step;

    MirLiteralExprPtr true_label;
    MirLiteralExprPtr false_label;
    MirLiteralExprPtr missing_label;
    public:
    MirBrInRangeInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr value, MirLiteralExprPtr lower, MirLiteralExprPtr upper, MirLiteralExprPtr step,
                     MirLiteralExprPtr true_label, MirLiteralExprPtr false_label, MirLiteralExprPtr missing_label);
    
    MirLiteralExprPtr get_value() const;
    MirLiteralExprPtr get_lower() const;
    MirLiteralExprPtr get_upper() const;
    MirLiteralExprPtr get_step() const;

    MirLiteralExprPtr get_true_label() const;
    MirLiteralExprPtr get_false_label() const;
    MirLiteralExprPtr get_missing_label() const;

    std::string to_string() const;  
};

class MirBrInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. br(label)
    MirLiteralExprPtr label;
    public:
    MirBrInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr label);
    
    MirLiteralExprPtr get_label() const;

    std::string to_string() const;  
};

class MirNopInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. nop()
    public:
    MirNopInst(InstructionStmtPtr instruction_stmt);
    
    std::string to_string() const;  
};

class MirHaltInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. halt()
    public:
    MirHaltInst(InstructionStmtPtr instruction_stmt);
    
    std::string to_string() const;  
};

class MirTrapInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. trap()
    public:
    MirTrapInst(InstructionStmtPtr instruction_stmt);
    
    std::string to_string() const;  
};

class MirTrapIfInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. trap_if(register)
    MirLiteralExprPtr condition;
    public:
    MirTrapIfInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr condition);
    
    MirLiteralExprPtr get_condition() const;

    std::string to_string() const;  
};

class MirTrapIfMissingInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. trap_if_missing(register)
    MirLiteralExprPtr condition;
    public:
    MirTrapIfMissingInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr condition);
    
    MirLiteralExprPtr get_condition() const;

    std::string to_string() const;  
};

class MirTraceInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. trace()
    MirLiteralExprPtr value;
    public:
    MirTraceInst(InstructionStmtPtr instruction_stmt);
    
    std::string to_string() const;  
};

class MirTraceIfInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. trace_if(register)
    MirLiteralExprPtr condition;
    public:
    MirTraceIfInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr condition);
    
    MirLiteralExprPtr get_condition() const;

    std::string to_string() const;  
};

class MirTraceIfMissingInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. trace_if_missing(register)
    MirLiteralExprPtr condition;
    public:
    MirTraceIfMissingInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr condition);
    
    MirLiteralExprPtr get_condition() const;

    std::string to_string() const;  
};

class MirPrintInst: public MirInstructionStmt{
    //Allowed canonical types for the instruction are:
    //1. print(register)
    MirLiteralExprPtr value;
    public:
    MirPrintInst(InstructionStmtPtr instruction_stmt, MirLiteralExprPtr value);
    
    MirLiteralExprPtr get_value() const;

    std::string to_string() const;  
};
}
}
}