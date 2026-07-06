#include "vm/vm.hpp"
#include "stream_macros.hpp"
#include <cstddef>
#include <cstdint>
#include <algorithm>
namespace Hazel{
namespace Snap{
//traceback_func_loc_idx, func_call_loc_idx and memory_offset_idx must be innitialized with every value as -1
//traceback_func_loc_idx is the idx in code where the function we call started. Like if we call func1() and func1 calls func2() then 
//traceback_func_loc_idx[0] is the idx in code where func1 started and traceback_func_loc_idx[1] is the idx in code where func2 started. 
//If we call func3() from func2() then traceback_func_loc_idx[2] is the idx in code where func3 started. If we return from func3() then traceback_func_loc_idx[2] is set to -1.
//If we return from func2() then traceback_func_loc_idx[1] is set to -1. If we return from func1() then traceback_func_loc_idx[0] is set to -1
//func_call_loc_idx iis the idx in code where the function we call is called. Like if we call func1() and func1 calls func2() then
//func_call_loc_idx[0] is the idx in code where func1() is called by global
//func_call_loc_idx[1] is the idx in code where func2() is called by func1. 
//If we call func3() from func2() then func_call_loc_idx[2] is the idx in code where func3() is called from func2(). 
//If we return from func3() then func_call_loc_idx[2] is set to -1. If we return from func2() then func_call_loc_idx[1] is set to -1. 
//If we return from func1() then func_call_loc_idx[0] is set to -1
//func_stack_size is the size of the stack for function calls. Like how many register are local to the function. Like if we call func1() then we insert the number of register local to func1 + parameter + return value register in func_stack_size[0] and so on
//All these can have the maximum size of max_function_call_depth. If we exceed that then we have a stack overflow and that is UB
//execute_cold_inst expects code = code + pc, frame_buffer = frame_buffer + curr_stack_offset, 
//traceback_func_loc_idx = traceback_func_loc_idx + curr_func_call_depth, 
//func_call_loc_idx = func_call_loc_idx + curr_func_call_depth, 
//func_stack_size = func_stack_size + curr_func_call_depth
//NOTE:-When function call happens we modify the frame_buffer, traceback_func_loc_idx, func_call_loc_idx and func_stack_size with the above mentioned offset.
//We revert the changes when we return. So we dont need to offset again and again when we access
static void __attribute__((noinline, cold)) execute_cold_inst(StreamValue* code, StreamValue* frame_buffer, std::int64_t* traceback_func_loc_idx,
                                                              std::int64_t* func_call_loc_idx, std::int64_t* func_stack_size) noexcept{}
std::int64_t execute_stream_vm(StreamValue* code, StreamValue* frame_buffer, std::int64_t* traceback_func_loc_idx,
                       std::int64_t* func_call_loc_idx, std::int64_t* func_stack_size) noexcept{
    void* dispatch[(std::uint64_t)Opcode::OP_OPCODE_COUNT];
    std::size_t pc = 0;
    
    /* ---- Loads ----------------------------------------- */
    INSERT(OP_PTR_LOAD_PTR);
    INSERT(OP_I64_LOAD_I64); INSERT(OP_I64_LOAD_FLOAT);
    INSERT(OP_FLOAT_LOAD_I64); INSERT(OP_FLOAT_LOAD_FLOAT);

    INSERT(OP_PTR_LOAD_PTRI);
    INSERT(OP_I64_LOAD_II64); INSERT(OP_FLOAT_LOAD_FLOATI);

    /* ---- Binary Arithmetic Instructions ----------------------------------*/
    INSERT(OP_I64FLOAT_ADD_I64FLOAT_I64FLOAT);
    INSERT(OP_I64FLOAT_SUB_I64FLOAT_I64FLOAT);
    INSERT(OP_I64_MUL_I64_I64); INSERT(OP_FLOAT_MUL_TRUNC_FLOAT_FLOAT); INSERT(OP_FLOAT_MUL_ROUND_FLOAT_FLOAT);
    INSERT(OP_I64FLOAT_DIV_I64FLOAT_I64FLOAT);
    INSERT(OP_I64FLOAT_REM_I64FLOAT_I64FLOAT);
    INSERT(OP_I64FLOAT_MIN_I64FLOAT_I64FLOAT);
    INSERT(OP_I64FLOAT_MAX_I64FLOAT_I64FLOAT);

    INSERT(OP_I64FLOAT_ADD_I64FLOAT_I64FLOATI);
    INSERT(OP_I64_MUL_I64_II64); INSERT(OP_FLOAT_MUL_TRUNC_FLOAT_FLOATI); INSERT(OP_FLOAT_MUL_ROUND_FLOAT_FLOATI);
    INSERT(OP_I64FLOAT_DIV_I64FLOAT_I64FLOATI);
    INSERT(OP_I64FLOAT_REM_I64FLOAT_I64FLOATI);
    INSERT(OP_I64FLOAT_MAX_I64FLOAT_I64FLOATI);
    INSERT(OP_I64FLOAT_MIN_I64FLOAT_I64FLOATI);

    INSERT(OP_I64FLOAT_SUB_I64FLOATI_I64FLOAT);
    INSERT(OP_I64FLOAT_DIV_I64FLOATI_I64FLOAT);
    INSERT(OP_I64FLOAT_REM_I64FLOATI_I64FLOAT);

    /* ---- Trinary Arithmetic Instructions ------- */
    INSERT(OP_I64_FMA_I64_I64_I64); INSERT(OP_FLOAT_FMA_FLOAT_FLOAT_FLOAT);

    INSERT(OP_I64_FMA_II64_I64_I64); INSERT(OP_FLOAT_FMA_FLOATI_FLOAT_FLOAT);
    INSERT(OP_I64_FMA_II64_I64_II64); INSERT(OP_FLOAT_FMA_FLOATI_FLOAT_FLOATI);
    INSERT(OP_I64_FMA_I64_I64_II64); INSERT(OP_FLOAT_FMA_FLOAT_FLOAT_FLOATI);

    /* ---- Binary Bitwise Instructions ----------- */
    INSERT(OP_I64_BIT_AND_I64_I64); INSERT(OP_I64_BIT_OR_I64_I64); INSERT(OP_I64_BIT_XOR_I64_I64);
    INSERT(OP_I64_BIT_XOR_I64_II64);
 
    // INSERT(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOAT);
    // INSERT(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOAT);
    // INSERT(OP_I64_GT_PTRI64FLOAT_PTRI64FLOAT);
    // INSERT(OP_I64_GE_PTRI64FLOAT_PTRI64FLOAT);
    // INSERT(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
    // INSERT(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
    // INSERT(OP_I64_IN_RANGE_PTR_PTR_PTR_I64);
    // INSERT(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64);

    // INSERT(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOATI);
    // INSERT(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOATI);
    // INSERT(OP_I64_GT_PTRI64FLOAT_PTRI64FLOATI);
    // INSERT(OP_I64_GE_PTRI64FLOAT_PTRI64FLOATI);
    // INSERT(OP_I64_GT_PTRI64FLOATI_PTRI64FLOAT);
    // INSERT(OP_I64_GE_PTRI64FLOATI_PTRI64FLOAT);
    // INSERT(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
    // INSERT(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
    // INSERT(OP_I64_IN_RANGE_PTR_PTR_PTR_I64I);
    // INSERT(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64I);

    // INSERT(OP_I64_ABS_I64); INSERT(OP_FLOAT_ABS_FLOAT);
    // INSERT(OP_FLOAT_CEIL_FLOAT);
    // INSERT(OP_FLOAT_FLOOR_FLOAT);
    // INSERT(OP_FLOAT_INTEGRAL_PART_FLOAT);
    // INSERT(OP_FLOAT_FRACTIONAL_PART_FLOAT);
    // INSERT(OP_FLOAT_ROUNDNEAREST_FLOAT);
    // INSERT(OP_FLOAT_ROUNDEVEN_FLOAT);
    // INSERT(OP_FLOAT_SQRT_FLOAT);
    // INSERT(OP_I64_HAS_VALUE);
    // INSERT(OP_I64_HAS_NO_VALUE);
    
    // INSERT(OP_BR_EQ_PTRI64FLOAT_PTRI64FLOAT);
    // INSERT(OP_BR_GT_PTRI64FLOAT_PTRI64FLOAT);
    // INSERT(OP_BR_OR_I64_I64);
    // INSERT(OP_BR_AND_I64_I64);
    // INSERT(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
    // INSERT(OP_BR_IN_RANGE_PTR_PTR_PTR_I64);
    
    // INSERT(OP_BR_EQ_I64FLOAT_I64FLOATI);
    // INSERT(OP_BR_GT_I64FLOAT_I64FLOATI);
    // INSERT(OP_BR_GT_I64FLOATI_I64FLOAT);
    // INSERT(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
    // INSERT(OP_BR_IN_RANGE_PTR_PTR_PTR_I64I);
 
    // INSERT(OP_BR);
    // INSERT(OP_BR_TRUE);

    // INSERT(OP_CALL);
    // INSERT(OP_EXTERN_CALL_PTR);
    // INSERT(OP_RET);
 
    
    // INSERT(OP_PTR_ALLOC_I64_I64_II64);
    // INSERT(OP_PTR_REALLOC_I64_I64_II64); 
    // INSERT(OP_FREE_PTR); 

    // INSERT(OP_I64_GET_PTR_I64);
    // INSERT(OP_FLOAT_GET_PTR_I64);
    // INSERT(OP_PTR_GET_PTR_I64);

    // INSERT(OP_I64_GET_PTR_II64);
    // INSERT(OP_FLOAT_GET_PTR_II64);
    // INSERT(OP_PTR_GET_PTR_II64);

    // INSERT(OP_PTR_PTROFFSET_ADD_PTR_I64);
    // INSERT(OP_PTR_PTROFFSET_SUB_PTR_I64);

    // INSERT(OP_PTR_PTROFFSET_ADD_PTR_II64);

    // INSERT(OP_SET_PTR_I64_I64FLOAT);
    // INSERT(OP_SET_PTR_I64_PTR);

    // INSERT(OP_SET_PTR_II64_I64FLOAT);
    // INSERT(OP_SET_PTR_II64_PTR);

    // INSERT(OP_I64_LEN_PTR);
    // INSERT(OP_I64_CAP_PTR);
    // INSERT(OP_I64_ELM_SIZE_PTR);
    // INSERT(OP_POP_PTR_I64);
    // INSERT(OP_PUSH_PTR_PTR_I64);
    // INSERT(OP_PUSH_PTR_I64FLOAT);
    // INSERT(OP_MEMCPY_PTR_PTR_I64);
    // INSERT(OP_MEMMOVE_PTR_PTR_I64);
    // INSERT(OP_I64_UNSIGNED_CMP_PTR_PTR_I64);
    
    // INSERT(OP_GC_SAFEPOINT);

    // INSERT(OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT); 
    // INSERT(OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT);  
    // INSERT(OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT);
    // INSERT(OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT);

    // INSERT(OP_PRINT_PTR);

    /* ---- No-ops / control -------------------------------------------- */
    INSERT(OP_NOP);
    INSERT(OP_HALT);
    INSERT(OP_TRAP);

    goto *dispatch[(std::uint64_t)code[0].value];

    /* ---- Loads ----------------------------------------- */
    OP_SIMPLE_LOAD_INST(OP_PTR_LOAD_PTR, frame_buffer[code[++pc].value]);
    OP_SIMPLE_LOAD_INST(OP_I64_LOAD_I64, frame_buffer[code[++pc].value]);
    OP_SIMPLE_LOAD_INST(OP_FLOAT_LOAD_FLOAT, frame_buffer[code[++pc].value]);
    _L_OP_I64_LOAD_FLOAT:{
        const auto value = frame_buffer[code[++pc].value];
        const auto ptr = frame_buffer + code[++pc].value;
        ptr->value = value.value/FIXED_POINT_FLOAT_SCALING_FACTOR;\
        ptr->type = ValueType::VT_I64;
        ptr->is_missing = value.is_missing;
        DISPATCH();\
    }
    _L_OP_FLOAT_LOAD_I64:{
        const auto value = frame_buffer[code[++pc].value];
        const auto ptr = frame_buffer + code[++pc].value;
        ptr->value = value.value*FIXED_POINT_FLOAT_SCALING_FACTOR;\
        ptr->type = ValueType::VT_FLOAT;
        ptr->is_missing = value.is_missing;
        DISPATCH();\
    }

    OP_SIMPLE_LOAD_INST(OP_PTR_LOAD_PTRI, code[++pc]);
    OP_SIMPLE_LOAD_INST(OP_I64_LOAD_II64, code[++pc]);
    OP_SIMPLE_LOAD_INST(OP_FLOAT_LOAD_FLOATI, code[++pc]);

    /* ---- Binary Arithmetic Instructions ----------------------------------*/
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_ADD_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value + rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_SUB_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value - rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_MUL_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value * rhs.value);
    OP_SIMPLE_BINARY_INST(OP_FLOAT_MUL_TRUNC_FLOAT_FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                          static_cast<int64_t>((static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs.value)) / FIXED_POINT_FLOAT_SCALING_FACTOR));
    _L_OP_FLOAT_MUL_ROUND_FLOAT_FLOAT:{
        const auto lhs = frame_buffer[code[++pc].value];
        const auto rhs = frame_buffer[code[++pc].value];
        const auto ptr = frame_buffer + code[++pc].value;
        __int128 product = static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs.value);
        __int128 result = (product >= 0)
                            ? (product + FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR
                            : (product - FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR;
        ptr->value = static_cast<int64_t>(result);
        ptr->type = lhs.type;
        ptr->is_missing = lhs.is_missing || rhs.is_missing;
        DISPATCH();
    }
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_DIV_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value / rhs.value);
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value % rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MIN_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], std::min(lhs.value, rhs.value));
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MAX_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], std::max(lhs.value, rhs.value));

    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_ADD_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value + rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_MUL_I64_II64, frame_buffer[code[++pc].value], code[++pc], lhs.value * rhs.value);
    OP_SIMPLE_BINARY_INST(OP_FLOAT_MUL_TRUNC_FLOAT_FLOATI, frame_buffer[code[++pc].value], code[++pc], 
                          static_cast<int64_t>((static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs.value)) / FIXED_POINT_FLOAT_SCALING_FACTOR));
    _L_OP_FLOAT_MUL_ROUND_FLOAT_FLOATI:{
        const auto lhs = frame_buffer[code[++pc].value];
        const auto rhs = code[++pc];
        const auto ptr = frame_buffer + code[++pc].value;
        __int128 product = static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs.value);
        __int128 result = (product >= 0)
                            ? (product + FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR
                            : (product - FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR;
        ptr->value = static_cast<int64_t>(result);
        ptr->type = lhs.type;
        ptr->is_missing = lhs.is_missing || rhs.is_missing;
        DISPATCH();
    }
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_DIV_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value / rhs.value);
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value % rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MAX_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], std::min(lhs.value, rhs.value));
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MIN_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], std::max(lhs.value, rhs.value));

    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_SUB_I64FLOATI_I64FLOAT, code[++pc], frame_buffer[code[++pc].value], lhs.value - rhs.value);
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_DIV_I64FLOATI_I64FLOAT, code[++pc], frame_buffer[code[++pc].value], std::min(lhs.value, rhs.value));
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOATI_I64FLOAT, code[++pc], frame_buffer[code[++pc].value], std::max(lhs.value, rhs.value));

    /* ---- Binary Bitwise Instructions ----------- */
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_AND_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value && rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_OR_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value || rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_XOR_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value ^ rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_XOR_I64_II64, frame_buffer[code[++pc].value], code[++pc], lhs.value ^ rhs.value);
    
     /* ---- No-ops / control -------------------------------------------- */
    COLD_GOTO_BLOCK(OP_NOP);
    _L_OP_HALT: {
        return -1;
    }
    _L_OP_TRAP: {
        return pc;
    }
}
}
}