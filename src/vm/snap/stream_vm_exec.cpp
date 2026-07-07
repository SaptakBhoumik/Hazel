#include "vm/vm.hpp"
#include "stream_macros.hpp"
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include "common_func.hpp"

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
static std::uint64_t __attribute__((noinline, cold)) execute_cold_inst(std::uint64_t pc, StreamValue* code, StreamValue* frame_buffer, std::int64_t* traceback_func_loc_idx,
                                                              std::int64_t* func_call_loc_idx, std::int64_t* func_stack_size) noexcept{
    void* dispatch[(std::uint64_t)Opcode::OP_OPCODE_COUNT];
    /* ---- Other Arithmetic Instructions ------- */
    INSERT(OP_I64_WEIGHTED_ADD_II64_I64_REG_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_REG_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_REG_WEIGHT); 
    INSERT(OP_I64_WEIGHTED_ADD_II64_II64_REG_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_REG_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_REG_WEIGHT); 
    INSERT(OP_I64_WEIGHTED_ADD_II64_I64_IMM_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_IMM_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_IMM_WEIGHT); 
    INSERT(OP_I64_WEIGHTED_ADD_II64_II64_IMM_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_IMM_WEIGHT); INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_IMM_WEIGHT);


    /* ---- Unary Instruction ------------------*/
    INSERT(OP_I64FLOAT_ABS_I64FLOAT);
    INSERT(OP_FLOAT_CEIL_FLOAT);
    INSERT(OP_FLOAT_FLOOR_FLOAT);
    INSERT(OP_FLOAT_INTEGRAL_PART_FLOAT);
    INSERT(OP_FLOAT_FRACTIONAL_PART_FLOAT);
    INSERT(OP_FLOAT_ROUNDNEAREST_FLOAT);
    INSERT(OP_FLOAT_ROUNDEVEN_FLOAT);
    INSERT(OP_FLOAT_SQRT_FLOAT);

    /* ---- Heap objects ---------------------------------------------*/
    INSERT(OP_PTR_ALLOC_I64_I64_II64);
    INSERT(OP_PTR_REALLOC_PTR_I64_I64_II64); 
    INSERT(OP_FREE_PTR); 
    INSERT(OP_POP_PTR_I64);
    INSERT(OP_PUSH_PTR_PTR_I64);
    INSERT(OP_PUSH_PTR_I64FLOAT);
    INSERT(OP_MEMCPY_PTR_PTR_I64);
    INSERT(OP_MEMMOVE_PTR_PTR_I64);
    INSERT(OP_I64_UNSIGNED_CMP_PTR_PTR_I64);

    goto *dispatch[(std::uint64_t)code[pc].value];
    /* ---- Other Arithmetic Instructions ------- */
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_I64_REG_WEIGHT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], weight.value*arg.value); 
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_REG_WEIGHT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], fixed_point_float_mul_trunc(arg.value, weight.value)); 
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_REG_WEIGHT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], fixed_point_float_mul_round(arg.value, weight.value)); 

    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_II64_REG_WEIGHT, code[++pc], frame_buffer[code[++pc].value], weight.value*arg.value); 
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_REG_WEIGHT, code[++pc], frame_buffer[code[++pc].value], fixed_point_float_mul_trunc(arg.value, weight.value)); 
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_REG_WEIGHT, code[++pc], frame_buffer[code[++pc].value], fixed_point_float_mul_round(arg.value, weight.value)); 

    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_I64_IMM_WEIGHT, frame_buffer[code[++pc].value], code[++pc], weight.value*arg.value); 
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_IMM_WEIGHT, frame_buffer[code[++pc].value], code[++pc], fixed_point_float_mul_trunc(arg.value, weight.value)); 
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_IMM_WEIGHT, frame_buffer[code[++pc].value], code[++pc], fixed_point_float_mul_round(arg.value, weight.value)); 

    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_II64_IMM_WEIGHT, code[++pc], code[++pc], weight.value*arg.value); 
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_IMM_WEIGHT, code[++pc], code[++pc], fixed_point_float_mul_trunc(arg.value, weight.value));
    OP_SIMPLE_COLD_REDUCED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_IMM_WEIGHT, code[++pc], code[++pc], fixed_point_float_mul_round(arg.value, weight.value));

    /* ---- Unary Instruction ------------------*/
    OP_SIMPLE_COLD_UNARY_INST(OP_I64FLOAT_ABS_I64FLOAT, frame_buffer[code[++pc].value], arg.type, std::abs(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_CEIL_FLOAT, frame_buffer[code[++pc].value], ValueType::VT_FLOAT, fixed_point_ceil(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_FLOOR_FLOAT, frame_buffer[code[++pc].value], ValueType::VT_FLOAT, fixed_point_floor(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_INTEGRAL_PART_FLOAT, frame_buffer[code[++pc].value], ValueType::VT_FLOAT, fixed_point_integral_part(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_FRACTIONAL_PART_FLOAT, frame_buffer[code[++pc].value], ValueType::VT_FLOAT, fixed_point_fractional_part(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_ROUNDNEAREST_FLOAT, frame_buffer[code[++pc].value], ValueType::VT_FLOAT, fixed_point_roundnearest(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_ROUNDEVEN_FLOAT, frame_buffer[code[++pc].value], ValueType::VT_FLOAT, fixed_point_roundeven(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_SQRT_FLOAT, frame_buffer[code[++pc].value], ValueType::VT_FLOAT, fixed_point_sqrt(arg.value), arg.is_missing);

    /* ---- Heap objects ---------------------------------------------*/
    _L_OP_PTR_ALLOC_I64_I64_II64:{
        const auto length = frame_buffer[code[++pc].value];
        const auto capacity = frame_buffer[code[++pc].value];
        const auto element_size = frame_buffer[code[++pc].value];
        const auto result_ptr = frame_buffer + code[++pc].value;
        result_ptr->type = ValueType::VT_PTR;
        result_ptr->is_missing = length.is_missing || capacity.is_missing || element_size.is_missing;
        if(result_ptr->is_missing){
            result_ptr->value = 0;
            result_ptr->capacity = 0;
            result_ptr->length = 0;
            result_ptr->element_size = 0;
        }
        else{
            result_ptr->value = (std::int64_t)malloc(capacity.value*element_size.value);
            if(result_ptr->value == 0){
                result_ptr->is_missing = true;
                result_ptr->capacity = 0;
                result_ptr->length = 0;
                result_ptr->element_size = 0;
            }
            result_ptr->capacity = capacity.value;
            result_ptr->length = length.value;
            result_ptr->element_size = element_size.value;
        }
        return pc;
    }
    _L_OP_PTR_REALLOC_PTR_I64_I64_II64:{
        const auto ptr = frame_buffer[code[++pc].value];
        const auto new_length = frame_buffer[code[++pc].value];
        const auto new_capacity = frame_buffer[code[++pc].value];
        const auto element_size = frame_buffer[code[++pc].value];
        const auto result_ptr = frame_buffer + code[++pc].value;
        result_ptr->type = ValueType::VT_PTR;
        result_ptr->is_missing = ptr.is_missing || new_length.is_missing || new_capacity.is_missing || element_size.is_missing;
        if(result_ptr->is_missing){
            result_ptr->value = 0;
            result_ptr->capacity = 0;
            result_ptr->length = 0;
            result_ptr->element_size = 0;
        }
        else{
            result_ptr->value = (std::int64_t)realloc((void*)ptr.value, new_capacity.value*element_size.value);
            if(result_ptr->value == 0){
                result_ptr->is_missing = true;
                result_ptr->capacity = 0;
                result_ptr->length = 0;
                result_ptr->element_size = 0;
            }
            result_ptr->capacity = new_capacity.value;
            result_ptr->length = new_length.value;
            result_ptr->element_size = element_size.value;
        }
        return pc;
    }
    _L_OP_FREE_PTR:{
        const auto ptr = frame_buffer[code[++pc].value];
        if(!ptr.is_missing && ptr.value != 0){
            free((void*)ptr.value);
        }
        return pc;
    }
    _L_OP_POP_PTR_I64:{
        const auto ptr = frame_buffer + code[++pc].value;
        const auto num_elements = frame_buffer[code[++pc].value];
        if(!ptr->is_missing && !num_elements.is_missing && num_elements.value > 0){
            ptr->length = std::max<std::int64_t>(0, ptr->length - num_elements.value);
        }
        return pc;
    }
    INSERT(OP_PUSH_PTR_PTR_I64);
    INSERT(OP_PUSH_PTR_I64FLOAT);
    _L_OP_MEMCPY_PTR_PTR_I64:{
        const auto dest = frame_buffer[code[++pc].value];
        const auto src = frame_buffer[code[++pc].value];
        const auto num_bytes = frame_buffer[code[++pc].value];
        if(!dest.is_missing && !src.is_missing && num_bytes.value > 0){
            memcpy((void*)dest.value, (void*)src.value, num_bytes.value*dest.element_size);
        }
        return pc;
    }
    _L_OP_MEMMOVE_PTR_PTR_I64:{
        const auto dest = frame_buffer[code[++pc].value];
        const auto src = frame_buffer[code[++pc].value];
        const auto num_bytes = frame_buffer[code[++pc].value];
        if(!dest.is_missing && !src.is_missing && num_bytes.value > 0){
            memmove((void*)dest.value, (void*)src.value, num_bytes.value*dest.element_size);
        }
        return pc;
    }
    _L_OP_I64_UNSIGNED_CMP_PTR_PTR_I64:{
        const auto dest = frame_buffer[code[++pc].value];
        const auto src = frame_buffer[code[++pc].value];
        const auto num_bytes = frame_buffer[code[++pc].value];
        const auto result = frame_buffer + code[++pc].value;
        result->type = ValueType::VT_I64;
        result->is_missing = dest.is_missing || src.is_missing || num_bytes.is_missing;
        if(result->is_missing){
            result->value = 0;
        }
        else{
            result->value = (std::int64_t)memcmp((void*)dest.value, (void*)src.value, num_bytes.value*dest.element_size);
        }
        return pc;
    }
}
//path is the array of pc values where we want to trace the execution via OP_TRACE. It is used for debugging
std::int64_t execute_stream_vm(StreamValue* code, StreamValue* frame_buffer, std::int64_t* traceback_func_loc_idx,
                               std::int64_t* func_call_loc_idx, std::int64_t* func_stack_size, std::vector<std::uint64_t>& path) noexcept{
    void* dispatch[(std::uint64_t)Opcode::OP_OPCODE_COUNT];
    std::uint64_t pc = 0;
    
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
    INSERT(OP_I64_DIV_I64_I64); INSERT(OP_FLOAT_DIV_FLOAT_FLOAT);
    INSERT(OP_I64FLOAT_REM_I64FLOAT_I64FLOAT);
    INSERT(OP_I64FLOAT_MIN_I64FLOAT_I64FLOAT);
    INSERT(OP_I64FLOAT_MAX_I64FLOAT_I64FLOAT);

    INSERT(OP_I64FLOAT_ADD_I64FLOAT_I64FLOATI);
    INSERT(OP_I64_MUL_I64_II64); INSERT(OP_FLOAT_MUL_TRUNC_FLOAT_FLOATI); INSERT(OP_FLOAT_MUL_ROUND_FLOAT_FLOATI);
    INSERT(OP_I64_DIV_I64_II64); INSERT(OP_FLOAT_DIV_FLOAT_FLOATI);
    INSERT(OP_I64FLOAT_REM_I64FLOAT_I64FLOATI);
    INSERT(OP_I64FLOAT_MAX_I64FLOAT_I64FLOATI);
    INSERT(OP_I64FLOAT_MIN_I64FLOAT_I64FLOATI);

    INSERT(OP_I64FLOAT_SUB_I64FLOATI_I64FLOAT);
    INSERT(OP_I64_DIV_II64_I64); INSERT(OP_FLOAT_DIV_FLOATI_FLOAT);
    INSERT(OP_I64FLOAT_REM_I64FLOATI_I64FLOAT);

    /* ---- Other Arithmetic Instructions ------- */
    INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_I64_REG_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_REG_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_REG_WEIGHT); 
    INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_II64_REG_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_REG_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_REG_WEIGHT); 
    INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_I64_IMM_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_IMM_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_IMM_WEIGHT); 
    INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_II64_IMM_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_IMM_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_IMM_WEIGHT);

    /* ---- Binary Bitwise Instructions ----------- */
    INSERT(OP_I64_BIT_AND_I64_I64); INSERT(OP_I64_BIT_OR_I64_I64); INSERT(OP_I64_BIT_XOR_I64_I64);
    INSERT(OP_I64_BIT_XOR_I64_II64);
 
    /* ---- Comparison Instruction ------------------*/
    INSERT(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOAT);
    INSERT(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOAT);
    INSERT(OP_I64_GT_PTRI64FLOAT_PTRI64FLOAT);
    INSERT(OP_I64_GE_PTRI64FLOAT_PTRI64FLOAT);
    INSERT(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
    INSERT(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
    INSERT(OP_I64_IN_RANGE_PTR_PTR_PTR_I64);
    INSERT(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64);

    INSERT(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOATI);
    INSERT(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOATI);
    INSERT(OP_I64_GT_PTRI64FLOAT_PTRI64FLOATI);
    INSERT(OP_I64_GE_PTRI64FLOAT_PTRI64FLOATI);
    INSERT(OP_I64_GT_PTRI64FLOATI_PTRI64FLOAT);
    INSERT(OP_I64_GE_PTRI64FLOATI_PTRI64FLOAT);
    INSERT(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
    INSERT(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
    INSERT(OP_I64_IN_RANGE_PTR_PTR_PTR_I64I);
    INSERT(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64I);

    /* ---- Unary Instruction ------------------*/
    INSERT_COLD(OP_I64FLOAT_ABS_I64FLOAT);
    INSERT_COLD(OP_FLOAT_CEIL_FLOAT);
    INSERT_COLD(OP_FLOAT_FLOOR_FLOAT);
    INSERT_COLD(OP_FLOAT_INTEGRAL_PART_FLOAT);
    INSERT_COLD(OP_FLOAT_FRACTIONAL_PART_FLOAT);
    INSERT_COLD(OP_FLOAT_ROUNDNEAREST_FLOAT);
    INSERT_COLD(OP_FLOAT_ROUNDEVEN_FLOAT);
    INSERT_COLD(OP_FLOAT_SQRT_FLOAT);
    INSERT(OP_I64_HAS_VALUE);
    INSERT(OP_I64_HAS_NO_VALUE);
    
    /* ---- Fused Compare-and-Branch(Takes in 2 or 4 operands + 3 branches(if branch,else branch and data missing branch)) ----------------------------------------*/
    INSERT(OP_BR_EQ_PTRI64FLOAT_PTRI64FLOAT);
    INSERT(OP_BR_GT_PTRI64FLOAT_PTRI64FLOAT);
    INSERT(OP_BR_OR_I64_I64);
    INSERT(OP_BR_AND_I64_I64);
    INSERT(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
    INSERT(OP_BR_IN_RANGE_PTR_PTR_PTR_I64);
    
    INSERT(OP_BR_EQ_I64FLOAT_I64FLOATI);
    INSERT(OP_BR_GT_I64FLOAT_I64FLOATI);
    INSERT(OP_BR_GT_I64FLOATI_I64FLOAT);
    INSERT(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
    INSERT(OP_BR_IN_RANGE_PTR_PTR_PTR_I64I);
 
    /*--------Regular branch-----*/
    INSERT(OP_BR);
    INSERT(OP_BR_TRUE);

    /* ---- Calls --------------------------------------- */
    // INSERT(OP_CALL);
    // INSERT(OP_EXTERN_CALL_PTR);
    // INSERT(OP_RET);
 
    /* ---- Heap objects: arrays ---------------------------------------------*/
    INSERT_COLD(OP_PTR_ALLOC_I64_I64_II64);
    INSERT_COLD(OP_PTR_REALLOC_PTR_I64_I64_II64); 
    INSERT_COLD(OP_FREE_PTR); 

    INSERT(OP_I64_GET_PTR_I64);
    INSERT(OP_FLOAT_GET_PTR_I64);
    INSERT(OP_PTR_GET_PTR_I64);

    INSERT(OP_I64_GET_PTR_II64);
    INSERT(OP_FLOAT_GET_PTR_II64);
    INSERT(OP_PTR_GET_PTR_II64);

    INSERT(OP_PTR_PTROFFSET_ADD_PTR_I64);
    INSERT(OP_PTR_PTROFFSET_ADD_PTR_II64);
    INSERT(OP_PTR_PTROFFSET_SUB_PTR_I64);

    INSERT(OP_SET_PTR_I64_I64FLOAT);
    INSERT(OP_SET_PTR_I64_PTR);

    INSERT(OP_SET_PTR_II64_I64FLOAT);
    INSERT(OP_SET_PTR_II64_PTR);

    INSERT(OP_I64_LEN_PTR);
    INSERT(OP_I64_CAP_PTR);
    INSERT(OP_I64_ELM_SIZE_PTR);
    INSERT_COLD(OP_POP_PTR_I64);
    INSERT_COLD(OP_PUSH_PTR_PTR_I64);
    INSERT_COLD(OP_PUSH_PTR_I64FLOAT);
    INSERT_COLD(OP_MEMCPY_PTR_PTR_I64);
    INSERT_COLD(OP_MEMMOVE_PTR_PTR_I64);
    INSERT_COLD(OP_I64_UNSIGNED_CMP_PTR_PTR_I64);
    
    /*----Time series related--------------------------------------*/
    INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT); 
    INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT);  
    INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT);
    INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT);

    /*-------Traceback--------------------------------*/
    INSERT_COLD(OP_PRINT_PTR);

    /* ---- No-ops / control -------------------------------------------- */
    INSERT(OP_NOP);
    INSERT(OP_HALT);
    INSERT(OP_TRAP);
    INSERT(OP_IF_TRUE_TRAP);
    INSERT(OP_IF_MISSING_TRAP);
    INSERT(OP_TRACE);

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
    OP_SIMPLE_DIV_REM_INST(OP_I64_DIV_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value / rhs.value);
    OP_SIMPLE_FLOAT_DIV_INST(OP_FLOAT_DIV_FLOAT_FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            static_cast<int64_t>(static_cast<__int128>(lhs.value) * FIXED_POINT_FLOAT_SCALING_FACTOR) / static_cast<__int128>(rhs.value));
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value % rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MIN_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], std::min(lhs.value, rhs.value));
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MAX_I64FLOAT_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], std::max(lhs.value, rhs.value));

    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_ADD_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value + rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_MUL_I64_II64, frame_buffer[code[++pc].value], code[++pc], lhs.value * rhs.value);
    OP_SIMPLE_BINARY_INST(OP_FLOAT_MUL_TRUNC_FLOAT_FLOATI, frame_buffer[code[++pc].value], code[++pc], 
                          static_cast<int64_t>((static_cast<__int128>(lhs.value) * FIXED_POINT_FLOAT_SCALING_FACTOR) / static_cast<__int128>(rhs.value)));
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
    OP_SIMPLE_DIV_REM_INST(OP_I64_DIV_I64_II64, frame_buffer[code[++pc].value], code[++pc], lhs.value / rhs.value);
    OP_SIMPLE_FLOAT_DIV_INST(OP_FLOAT_DIV_FLOAT_FLOATI, frame_buffer[code[++pc].value], code[++pc], 
                             static_cast<int64_t>((static_cast<__int128>(lhs.value) * FIXED_POINT_FLOAT_SCALING_FACTOR) / static_cast<__int128>(rhs.value)));
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value % rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MIN_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], std::min(lhs.value, rhs.value));
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MAX_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], std::max(lhs.value, rhs.value));

    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_SUB_I64FLOATI_I64FLOAT, code[++pc], frame_buffer[code[++pc].value], lhs.value - rhs.value);
    OP_SIMPLE_DIV_REM_INST(OP_I64_DIV_II64_I64, code[++pc], frame_buffer[code[++pc].value], lhs.value / rhs.value);
    OP_SIMPLE_FLOAT_DIV_INST(OP_FLOAT_DIV_FLOATI_FLOAT, code[++pc], frame_buffer[code[++pc].value], 
                             static_cast<int64_t>((static_cast<__int128>(lhs.value) * FIXED_POINT_FLOAT_SCALING_FACTOR) / static_cast<__int128>(rhs.value)));
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOATI_I64FLOAT, code[++pc], frame_buffer[code[++pc].value], lhs.value % rhs.value);

    /* ---- Binary Bitwise Instructions ----------- */
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_AND_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value & rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_OR_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value | rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_XOR_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value ^ rhs.value);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_XOR_I64_II64, frame_buffer[code[++pc].value], code[++pc], lhs.value ^ rhs.value);

    /* ---- Comparison Instruction ------------------*/
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value == rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value != rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GT_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value > rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GE_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value >= rhs.value);
    OP_SIMPLE_IN_RANGE_INST(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value));
    OP_SIMPLE_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value));
    OP_SIMPLE_IN_RANGE_INST(OP_I64_IN_RANGE_PTR_PTR_PTR_I64, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size));//NOTE:Always use the element size of arg1
    OP_SIMPLE_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size));//NOTE:Always use the element size of arg1

    OP_SIMPLE_CMP_BINARY_INST(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value == rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value != rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GT_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value > rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GE_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value >= rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GT_PTRI64FLOATI_PTRI64FLOAT, code[++pc], frame_buffer[code[++pc].value], lhs.value > rhs.value);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GE_PTRI64FLOATI_PTRI64FLOAT, code[++pc], frame_buffer[code[++pc].value], lhs.value >= rhs.value);
    OP_SIMPLE_IN_RANGE_INST(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], code[++pc], 
                            is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value));
    OP_SIMPLE_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], code[++pc], 
                            !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value));
    OP_SIMPLE_IN_RANGE_INST(OP_I64_IN_RANGE_PTR_PTR_PTR_I64I, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], code[++pc], 
                            is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size));//NOTE:Always use the element size of arg1
    OP_SIMPLE_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64I, 
                            frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                            frame_buffer[code[++pc].value], code[++pc], 
                            !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size));//NOTE:Always use the element size of arg1
    
    /* ---- Unary Instruction ------------------*/
    OP_SIMPLE_UNARY_INST(OP_I64_HAS_VALUE, frame_buffer[code[++pc].value], ValueType::VT_I64, arg.is_missing, false);
    OP_SIMPLE_UNARY_INST(OP_I64_HAS_NO_VALUE, frame_buffer[code[++pc].value], ValueType::VT_I64, arg.is_missing, false);

    /* ---- Fused Compare-and-Branch(Takes in 2 or 4 operands + 3 branches(if branch,else branch and data missing branch)) ----------------------------------------*/
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_EQ_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value == rhs.value);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_GT_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value > rhs.value);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_OR_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value || rhs.value);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_AND_I64_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], lhs.value && rhs.value);
    OP_SIMPLE_BR_IN_RANGE_INST(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT, 
                               frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                               frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                               is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value));
    OP_SIMPLE_BR_IN_RANGE_INST(OP_BR_IN_RANGE_PTR_PTR_PTR_I64, 
                               frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                               frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                               is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size));//NOTE:Always use the element size of arg1
    
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_EQ_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value == rhs.value);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_GT_I64FLOAT_I64FLOATI, frame_buffer[code[++pc].value], code[++pc], lhs.value > rhs.value);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_GT_I64FLOATI_I64FLOAT, code[++pc], frame_buffer[code[++pc].value], lhs.value > rhs.value);
    OP_SIMPLE_BR_IN_RANGE_INST(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI, 
                               frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                               frame_buffer[code[++pc].value], code[++pc], 
                               is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value));
    OP_SIMPLE_BR_IN_RANGE_INST(OP_BR_IN_RANGE_PTR_PTR_PTR_I64I, 
                               frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], 
                               frame_buffer[code[++pc].value], code[++pc], 
                               is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size));//NOTE:Always use the element size of arg1
    
    /*--------Regular branch-----*/
    _L_OP_BR:{
        pc = code[++pc].value;
        goto *dispatch[(std::uint64_t)code[pc].value];
    }
    _L_OP_BR_TRUE:{
        const auto condition = frame_buffer[code[++pc].value];
        const auto label1 = frame_buffer[code[++pc].value].value;
        const auto label2 = frame_buffer[code[++pc].value].value;
        const auto label3 = frame_buffer[code[++pc].value].value;
        if(condition.is_missing){GOTO(label3);}
        else if(condition.value){GOTO(label1);}
        else{GOTO(label2);}
    }

    /* ---- Calls --------------------------------------- */

    /* ---- Heap objects: arrays ---------------------------------------------*/
    OP_SIMPLE_GET_INT64FLOAT_INST(OP_I64_GET_PTR_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], ValueType::VT_I64);
    OP_SIMPLE_GET_INT64FLOAT_INST(OP_FLOAT_GET_PTR_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], ValueType::VT_I64);
    OP_SIMPLE_GET_PTR_INST(OP_PTR_GET_PTR_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value]);

    OP_SIMPLE_GET_INT64FLOAT_INST(OP_I64_GET_PTR_II64, frame_buffer[code[++pc].value], code[++pc], ValueType::VT_I64);
    OP_SIMPLE_GET_INT64FLOAT_INST(OP_FLOAT_GET_PTR_II64, frame_buffer[code[++pc].value], code[++pc], ValueType::VT_I64);
    OP_SIMPLE_GET_PTR_INST(OP_PTR_GET_PTR_II64, frame_buffer[code[++pc].value], code[++pc]);

    OP_SIMPLE_PTR_OFFSET_INST(OP_PTR_PTROFFSET_ADD_PTR_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], ptr.value + offset.value*ptr.element_size);
    OP_SIMPLE_PTR_OFFSET_INST(OP_PTR_PTROFFSET_SUB_PTR_I64, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], ptr.value - offset.value*ptr.element_size);

    OP_SIMPLE_PTR_OFFSET_INST(OP_PTR_PTROFFSET_ADD_PTR_II64, frame_buffer[code[++pc].value], code[++pc], ptr.value + offset.value*ptr.element_size);

    OP_SIMPLE_SET_INT64FLOAT_INST(OP_SET_PTR_I64_I64FLOAT, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], frame_buffer[code[++pc].value]);
    OP_SIMPLE_SET_PTR_INST(OP_SET_PTR_I64_PTR, frame_buffer[code[++pc].value], frame_buffer[code[++pc].value], frame_buffer[code[++pc].value]);

    OP_SIMPLE_SET_INT64FLOAT_INST(OP_SET_PTR_II64_I64FLOAT, frame_buffer[code[++pc].value], code[++pc], frame_buffer[code[++pc].value]);
    OP_SIMPLE_SET_PTR_INST(OP_SET_PTR_II64_PTR, frame_buffer[code[++pc].value], code[++pc], frame_buffer[code[++pc].value]);

    OP_SIMPLE_PTR_GETTER_METHOD_INST(OP_I64_LEN_PTR,ptr.length);
    OP_SIMPLE_PTR_GETTER_METHOD_INST(OP_I64_CAP_PTR,ptr.capacity);
    OP_SIMPLE_PTR_GETTER_METHOD_INST(OP_I64_ELM_SIZE_PTR,ptr.element_size);

    /* ---- No-ops / control -------------------------------------------- */
    _L_OP_NOP:{
        DISPATCH();
    }
    _L_OP_HALT:{
        return -1;
    }
    _L_OP_TRAP:{
        return pc;
    }
    _L_OP_IF_TRUE_TRAP:{
        const auto condition = frame_buffer[code[++pc].value];
        if(condition.is_missing || !condition.value){return pc;}
        else{DISPATCH();}
    }
    _L_OP_IF_MISSING_TRAP:{
        const auto condition = frame_buffer[code[++pc].value];
        if(!condition.is_missing){return pc;}
        else{DISPATCH();}
    }
    _L_OP_TRACE:{
        path.push_back(pc);
        DISPATCH();
    }
    ___L_COLD_LABEL:{
        pc = execute_cold_inst(pc, code, frame_buffer, traceback_func_loc_idx, func_call_loc_idx, func_stack_size);//pc on the last argument of the instruction or on the instruction if it has no argument
        DISPATCH();
    }
}
}
}