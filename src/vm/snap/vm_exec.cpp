#include "vm/value.hpp"
#include "vm/vm.hpp"
#include "macros.hpp"
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <iostream>
#include "common_func.hpp"

#if defined(__GNUC__) && !defined(__clang__)
#define VM_NOCLONE __attribute__((noclone))
#else
#define VM_NOCLONE
#endif

namespace Hazel{
namespace Snap{
//traceback_func_loc_idx, ret_loc_idx and memory_offset_idx must be innitialized with every value as -1
//traceback_func_loc_idx is the idx in code where the function we call started. Like if we call func1() and func1 calls func2() then 
//traceback_func_loc_idx[0] is the idx in code where func1 started and traceback_func_loc_idx[1] is the idx in code where func2 started. 
//If we call func3() from func2() then traceback_func_loc_idx[2] is the idx in code where func3 started. If we return from func3() then traceback_func_loc_idx[2] is set to -1.
//If we return from func2() then traceback_func_loc_idx[1] is set to -1. If we return from func1() then traceback_func_loc_idx[0] is set to -1
//ret_loc_idx is the idx in code where the call instruction ends. We are supposed to return to the index after this
//func_stack_off is the offset of the stack for function calls. Like how many register are local to the function. Like if we call func1() then we insert the number of register local to func1 + parameter + return value register in func_stack_off[0] and so on
//All these can have the maximum size of max_function_call_depth. If we exceed that then we have a stack overflow and that is UB
//execute_cold_inst expects code = code + pc, frame_buffer = frame_buffer + curr_stack_offset, 
//traceback_func_loc_idx = traceback_func_loc_idx + curr_func_call_depth, 
//ret_loc_idx = ret_loc_idx + curr_func_call_depth, 
//func_stack_off = func_stack_off + curr_func_call_depth
//NOTE:-When function call happens we modify the frame_buffer, traceback_func_loc_idx, ret_loc_idx and func_stack_size with the above mentioned offset.
//We revert the changes when we return. So we dont need to offset again and again when we access
static void* cold_dispatch[(std::uint64_t)Opcode::OP_OPCODE_COUNT];
static void* hot_dispatch[(std::uint64_t)Opcode::OP_OPCODE_COUNT];

VM_NOCLONE Value* __attribute__((noinline, used, cold)) execute_cold_inst(Value* code, Value* frame_buffer) noexcept{
    if(code == nullptr){
        //The first call(from same thread as the original call) to this function will have code = nullptr. So we need to initialize the cold_dispatch table
        /* ---- Other Arithmetic Instructions ------- */
        COLD_INSERT(OP_I64_WEIGHTED_ADD_II64_I64_REG_WEIGHT); COLD_INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_REG_WEIGHT); 
        COLD_INSERT(OP_I64_WEIGHTED_ADD_II64_II64_REG_WEIGHT);COLD_INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_REG_WEIGHT); 
        COLD_INSERT(OP_I64_WEIGHTED_ADD_II64_I64_IMM_WEIGHT); COLD_INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_IMM_WEIGHT); 
        COLD_INSERT(OP_I64_WEIGHTED_ADD_II64_II64_IMM_WEIGHT); COLD_INSERT(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_IMM_WEIGHT);
        // INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_REG_WEIGHT);
        // INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_REG_WEIGHT); 
        // INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_IMM_WEIGHT);  
        // INSERT(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_IMM_WEIGHT);
        /* ---- Comparison Instruction ------------------*/
        COLD_INSERT(OP_I64_IN_RANGE_PTR_PTR_PTR_I64);
        COLD_INSERT(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64);

        COLD_INSERT(OP_I64_IN_RANGE_PTR_PTR_PTR_II64);
        COLD_INSERT(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_II64);

        /* ---- Unary Instruction ------------------*/
        COLD_INSERT(OP_I64FLOAT_ABS_I64FLOAT);
        COLD_INSERT(OP_FLOAT_CEIL_FLOAT);
        COLD_INSERT(OP_FLOAT_FLOOR_FLOAT);
        COLD_INSERT(OP_FLOAT_INTEGRAL_PART_FLOAT);
        COLD_INSERT(OP_FLOAT_FRACTIONAL_PART_FLOAT);
        COLD_INSERT(OP_FLOAT_ROUNDNEAREST_FLOAT);
        COLD_INSERT(OP_FLOAT_ROUNDEVEN_FLOAT);
        COLD_INSERT(OP_FLOAT_SQRT_FLOAT);

        /* ---- Fused Compare-and-Branch(Takes in 2 or 4 operands + 3 branches(if branch,else branch and data missing branch)) ----------------------------------------*/
        COLD_INSERT(OP_BR_IN_RANGE_PTR_PTR_PTR_I64);
        COLD_INSERT(OP_BR_IN_RANGE_PTR_PTR_PTR_II64);
        
        /* ---- Heap objects ---------------------------------------------*/
        COLD_INSERT(OP_PTR_ALLOC_I64_I64_II64);
        COLD_INSERT(OP_PTR_REALLOC_PTR_I64_I64); 
        COLD_INSERT(OP_FREE_PTR); 
        COLD_INSERT(OP_I64_GET_PTR_I64);
        COLD_INSERT(OP_FLOAT_GET_PTR_I64);
        COLD_INSERT(OP_PTR_GET_PTR_I64);
        COLD_INSERT(OP_PTR_PTROFFSET_ADD_PTR_I64);
        COLD_INSERT(OP_PTR_PTROFFSET_ADD_PTR_II64);
        COLD_INSERT(OP_SET_PTR_I64_I64FLOAT);
        COLD_INSERT(OP_SET_PTR_I64_PTR);
        COLD_INSERT(OP_PTR_PTROFFSET_SUB_PTR_I64);
        COLD_INSERT(OP_I64_LEN_PTR);
        COLD_INSERT(OP_I64_CAP_PTR);
        COLD_INSERT(OP_I64_ELM_SIZE_PTR);
        COLD_INSERT(OP_POP_PTR_I64);
        COLD_INSERT(OP_PUSH_PTR_PTR_I64_II64);
        COLD_INSERT(OP_PUSH_PTR_I64FLOAT_II64);
        COLD_INSERT(OP_PUSH_PTR_PTR_VALUE_II64);
        COLD_INSERT(OP_MEMCPY_PTR_PTR_I64);
        COLD_INSERT(OP_MEMMOVE_PTR_PTR_I64);
        COLD_INSERT(OP_I64_UNSIGNED_CMP_PTR_PTR_I64);

        /*----Time series related--------------------------------------*/
        COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_I64_I64_I64FLOAT); 
        COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_I64_I64_I64FLOAT);  
        COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_II64_I64_I64FLOAT);
        COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_II64_I64_I64FLOAT);

        COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT); 
        COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT);  
        COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT);
        COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT);

        /*-------Traceback--------------------------------*/
        COLD_INSERT(OP_PRINT_PTR);

        return nullptr;
    }

    goto *(void*)code->capacity; //capacity is used to store the address of the computed goto label for cold dispatch.

    /* ---- Other Arithmetic Instructions ------- */
    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_I64_REG_WEIGHT, frame_buffer[(++code)->value], acc.value, frame_buffer[(++code)->value], weight.value*arg.value, acc.is_missing, weight.is_missing || arg.is_missing, ValueType::VT_I64); 
    // OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_REG_WEIGHT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], fixed_point_float_mul_trunc(arg.value, weight.value)); 
    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_REG_WEIGHT, frame_buffer[(++code)->value], acc.value, frame_buffer[(++code)->value], fixed_point_float_mul_round(arg.value, weight.value), acc.is_missing, weight.is_missing || arg.is_missing, ValueType::VT_FLOAT); 

    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_II64_REG_WEIGHT, (++code)->value, acc, frame_buffer[(++code)->value], weight.value*arg.value, false, weight.is_missing || arg.is_missing, ValueType::VT_I64); 
    // OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_REG_WEIGHT, *(++code), frame_buffer[(++code)->value], fixed_point_float_mul_trunc(arg.value, weight.value)); 
    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_REG_WEIGHT, (++code)->value, acc, frame_buffer[(++code)->value], fixed_point_float_mul_round(arg.value, weight.value), false, weight.is_missing || arg.is_missing, ValueType::VT_FLOAT); 

    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_I64_IMM_WEIGHT, frame_buffer[(++code)->value], acc.value, (++code)->value, weight*arg.value, acc.is_missing, arg.is_missing, ValueType::VT_I64); 
    // OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_IMM_WEIGHT, frame_buffer[(++code)->value], *(++code), fixed_point_float_mul_trunc(arg.value, weight.value)); 
    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_IMM_WEIGHT, frame_buffer[(++code)->value], acc.value, (++code)->value, fixed_point_float_mul_round(arg.value, weight), acc.is_missing, arg.is_missing, ValueType::VT_FLOAT); 

    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_I64_WEIGHTED_ADD_II64_II64_IMM_WEIGHT, (++code)->value, acc, (++code)->value, weight*arg.value, false, arg.is_missing, ValueType::VT_I64); 
    // OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_IMM_WEIGHT, *(++code), *(++code), fixed_point_float_mul_trunc(arg.value, weight.value));
    OP_SIMPLE_COLD_WEIGHED_ADD_INST(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_IMM_WEIGHT, (++code)->value, acc, (++code)->value, fixed_point_float_mul_round(arg.value, weight), false, arg.is_missing, ValueType::VT_FLOAT);

    /* ---- Comparison Instruction ------------------*/
    OP_SIMPLE_COLD_IN_RANGE_INST(OP_I64_IN_RANGE_PTR_PTR_PTR_I64, 
                                 frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                 frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                 is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size),
                                 arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4.is_missing);//NOTE:Always use the element size of arg1
    OP_SIMPLE_COLD_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64, 
                                 frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                 frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                 !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size),
                                 arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4.is_missing);//NOTE:Always use the element size of arg1
    
    OP_SIMPLE_COLD_IN_RANGE_INST(OP_I64_IN_RANGE_PTR_PTR_PTR_II64, 
                                 frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                 frame_buffer[(++code)->value], (++code)->value, 
                                 is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4*arg1.element_size),
                                 arg1.is_missing || arg2.is_missing || arg3.is_missing);//NOTE:Always use the element size of arg1
    OP_SIMPLE_COLD_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_II64, 
                                 frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                 frame_buffer[(++code)->value], (++code)->value, 
                                 !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4*arg1.element_size),
                                 arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4);//NOTE:Always use the element size of arg1
    
    /* ---- Unary Instruction ------------------*/
    OP_SIMPLE_COLD_UNARY_INST(OP_I64FLOAT_ABS_I64FLOAT, frame_buffer[(++code)->value], arg.type, std::abs(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_CEIL_FLOAT, frame_buffer[(++code)->value], ValueType::VT_FLOAT, fixed_point_ceil(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_FLOOR_FLOAT, frame_buffer[(++code)->value], ValueType::VT_FLOAT, fixed_point_floor(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_INTEGRAL_PART_FLOAT, frame_buffer[(++code)->value], ValueType::VT_FLOAT, fixed_point_integral_part(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_FRACTIONAL_PART_FLOAT, frame_buffer[(++code)->value], ValueType::VT_FLOAT, fixed_point_fractional_part(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_ROUNDNEAREST_FLOAT, frame_buffer[(++code)->value], ValueType::VT_FLOAT, fixed_point_roundnearest(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_ROUNDEVEN_FLOAT, frame_buffer[(++code)->value], ValueType::VT_FLOAT, fixed_point_roundeven(arg.value), arg.is_missing);
    OP_SIMPLE_COLD_UNARY_INST(OP_FLOAT_SQRT_FLOAT, frame_buffer[(++code)->value], ValueType::VT_FLOAT, fixed_point_sqrt(arg.value), arg.is_missing || arg.value < 0);
    
    /* ---- Fused Compare-and-Branch(Takes in 2 or 4 operands + 3 branches(if branch,else branch and data missing branch)) ----------------------------------------*/
    OP_SIMPLE_COLD_BR_IN_RANGE_INST(OP_BR_IN_RANGE_PTR_PTR_PTR_I64, 
                                    frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                    frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                    is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value*arg1.element_size),
                                    arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4.is_missing);//NOTE:Always use the element size of arg1
    OP_SIMPLE_COLD_BR_IN_RANGE_INST(OP_BR_IN_RANGE_PTR_PTR_PTR_II64, 
                                    frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                                    frame_buffer[(++code)->value], (++code)->value, 
                                    is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4*arg1.element_size),
                                    arg1.is_missing || arg2.is_missing || arg3.is_missing);//NOTE:Always use the element size of arg1
    
    /* ---- Heap objects ---------------------------------------------*/
    _L_OP_PTR_ALLOC_I64_I64_II64:{
        const auto length = frame_buffer[(++code)->value];
        const auto capacity = frame_buffer[(++code)->value];
        const auto element_size = *(++code);
        const auto result_ptr = frame_buffer + (++code)->value;
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
            else{
                result_ptr->capacity = capacity.value;
                result_ptr->length = length.value;
                result_ptr->element_size = element_size.value;
                memset((void*)result_ptr->value, 0, capacity.value*element_size.value);
            }
        }
        return code;
    }
    _L_OP_PTR_REALLOC_PTR_I64_I64:{
        const auto ptr = frame_buffer[(++code)->value];
        const auto new_length = frame_buffer[(++code)->value];
        const auto new_capacity = frame_buffer[(++code)->value];
        const auto element_size = ptr.element_size;
        const auto result_ptr = frame_buffer + (++code)->value;
        result_ptr->type = ValueType::VT_PTR;
        result_ptr->is_missing = ptr.is_missing || new_length.is_missing || new_capacity.is_missing ;
        if(result_ptr->is_missing){
            result_ptr->value = 0;
            result_ptr->capacity = 0;
            result_ptr->length = 0;
            result_ptr->element_size = 0;
        }
        else{
            result_ptr->value = (std::int64_t)realloc((void*)ptr.value, new_capacity.value*element_size);
            if(result_ptr->value == 0){
                result_ptr->is_missing = true;
                result_ptr->capacity = 0;
                result_ptr->length = 0;
                result_ptr->element_size = 0;
            }
            else{
                //Zero out new memory if capacity increased or length shrinked
                if(new_capacity.value > ptr.capacity){
                    memset((void*)(result_ptr->value + ptr.capacity*element_size), 0, (new_capacity.value - ptr.capacity)*element_size);
                }
                if(new_length.value < ptr.length){
                    const auto zero_end = std::min(ptr.length, new_capacity.value);
                    if(zero_end > new_length.value){
                        memset((void*)(result_ptr->value + new_length.value*element_size), 0, (zero_end - new_length.value)*element_size);
                    }
                }
                result_ptr->capacity = new_capacity.value;
                result_ptr->length = new_length.value;
                result_ptr->element_size = element_size;
            }
        }
        return code;
    }
    _L_OP_FREE_PTR:{
        const auto ptr = frame_buffer[(++code)->value];
        if(!ptr.is_missing && ptr.value != 0){
            free((void*)ptr.value);
        }
        return code;
    }

    OP_SIMPLE_COLD_GET_INT64FLOAT_INST(OP_I64_GET_PTR_I64, frame_buffer[(++code)->value], ValueType::VT_I64);
    OP_SIMPLE_COLD_GET_INT64FLOAT_INST(OP_FLOAT_GET_PTR_I64, frame_buffer[(++code)->value], ValueType::VT_I64);
    OP_SIMPLE_COLD_GET_PTR_INST(OP_PTR_GET_PTR_I64, frame_buffer[(++code)->value]);

    OP_SIMPLE_COLD_PTR_OFFSET_INST(OP_PTR_PTROFFSET_ADD_PTR_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], offset.value, ptr.value + offset.value*ptr.element_size, ptr.is_missing || offset.is_missing);
    OP_SIMPLE_COLD_PTR_OFFSET_INST(OP_PTR_PTROFFSET_SUB_PTR_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], offset.value, ptr.value - offset.value*ptr.element_size, ptr.is_missing || offset.is_missing);
    OP_SIMPLE_COLD_PTR_OFFSET_INST(OP_PTR_PTROFFSET_ADD_PTR_II64, frame_buffer[(++code)->value], (++code)->value, offset, ptr.value + offset*ptr.element_size, ptr.is_missing);

    OP_SIMPLE_COLD_SET_INT64FLOAT_INST(OP_SET_PTR_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value]);
    OP_SIMPLE_COLD_SET_PTR_INST(OP_SET_PTR_I64_PTR, frame_buffer[(++code)->value], frame_buffer[(++code)->value]);

    OP_SIMPLE_COLD_PTR_GETTER_METHOD_INST(OP_I64_LEN_PTR,ptr.length);
    OP_SIMPLE_COLD_PTR_GETTER_METHOD_INST(OP_I64_CAP_PTR,ptr.capacity);
    OP_SIMPLE_COLD_PTR_GETTER_METHOD_INST(OP_I64_ELM_SIZE_PTR,ptr.element_size);
    _L_OP_POP_PTR_I64:{
        const auto ptr = frame_buffer + (++code)->value;
        const auto num_elements = frame_buffer[(++code)->value];
        if(!ptr->is_missing && !num_elements.is_missing && num_elements.value > 0){
            ptr->length = std::max<std::int64_t>(0, ptr->length - num_elements.value);
        }
        return code;
    }
    _L_OP_PUSH_PTR_PTR_I64_II64:{
        auto dest_ptr = frame_buffer + (++code)->value;
        const auto src_ptr = frame_buffer[(++code)->value];
        const auto num_elements = frame_buffer[(++code)->value];
        const auto capacity_increase = (++code)->value;
        if(dest_ptr->is_missing || src_ptr.is_missing || num_elements.is_missing || num_elements.value <= 0 || dest_ptr->element_size*num_elements.value > src_ptr.length*src_ptr.element_size){}
        else{
            if(dest_ptr->length + num_elements.value > dest_ptr->capacity){
                const auto new_capacity = dest_ptr->capacity + capacity_increase + num_elements.value;
                const auto new_ptr = (std::int64_t)realloc((void*)dest_ptr->value, new_capacity*dest_ptr->element_size);
                if(new_ptr == 0){
                    return code;
                }
                else{
                    dest_ptr->value = new_ptr;
                    dest_ptr->capacity = new_capacity;
                }
            }
            memcpy((void*)(dest_ptr->value + dest_ptr->length*dest_ptr->element_size), (void*)src_ptr.value, num_elements.value*dest_ptr->element_size);
            dest_ptr->length += num_elements.value;
        }
        return code;
    }
    _L_OP_PUSH_PTR_I64FLOAT_II64:{
        auto dest_ptr = frame_buffer + (++code)->value;
        const auto value = frame_buffer[(++code)->value];
        const auto capacity_increase = (++code)->value;
        if(dest_ptr->is_missing || value.is_missing){}
        else{
            if(dest_ptr->length*dest_ptr->element_size + sizeof(std::int64_t) > dest_ptr->capacity*dest_ptr->element_size){
                const auto new_capacity = dest_ptr->capacity + capacity_increase + sizeof(std::int64_t)/dest_ptr->element_size;
                const auto new_ptr = (std::int64_t)realloc((void*)dest_ptr->value, new_capacity*dest_ptr->element_size);
                if(new_ptr == 0){
                    return code;
                }
                else{
                    dest_ptr->value = new_ptr;
                    dest_ptr->capacity = new_capacity;
                }
            }
            *(std::int64_t*)(dest_ptr->value + dest_ptr->length*dest_ptr->element_size) = value.value;
            dest_ptr->length += sizeof(std::int64_t)/dest_ptr->element_size;
        }
        return code;
        
    }
    _L_OP_PUSH_PTR_PTR_VALUE_II64:{
        auto dest_ptr = frame_buffer + (++code)->value;
        const auto value = frame_buffer[(++code)->value];
        const auto capacity_increase = (++code)->value;
        if(dest_ptr->is_missing || value.is_missing){}
        else{
            if(dest_ptr->length*dest_ptr->element_size + 3*sizeof(std::int64_t) + sizeof(std::int16_t) > dest_ptr->capacity*dest_ptr->element_size){
                const auto new_capacity = dest_ptr->capacity + capacity_increase + (3*sizeof(std::int64_t) + sizeof(std::int16_t))/dest_ptr->element_size;
                const auto new_ptr = (std::int64_t)realloc((void*)dest_ptr->value, new_capacity*dest_ptr->element_size);
                if(new_ptr == 0){
                    return code;
                }
                else{
                    dest_ptr->value = new_ptr;
                    dest_ptr->capacity = new_capacity;
                }
            }
            *(std::int64_t*)(dest_ptr->value + dest_ptr->length*dest_ptr->element_size) = value.value;
            *(std::int64_t*)(dest_ptr->value + dest_ptr->length*dest_ptr->element_size + sizeof(std::int64_t)) = value.capacity;
            *(std::int64_t*)(dest_ptr->value + dest_ptr->length*dest_ptr->element_size + 2*sizeof(std::int64_t)) = value.length;
            *(std::int16_t*)(dest_ptr->value + dest_ptr->length*dest_ptr->element_size + 2*sizeof(std::int64_t)+sizeof(std::int16_t)) = value.element_size;
            dest_ptr->length += (3*sizeof(std::int64_t) + sizeof(std::int16_t))/dest_ptr->element_size;
        }
        return code;
        
    }
    _L_OP_MEMCPY_PTR_PTR_I64:{
        const auto dest = frame_buffer[(++code)->value];
        const auto src = frame_buffer[(++code)->value];
        const auto no_of_item = frame_buffer[(++code)->value];
        if(!dest.is_missing && !no_of_item.is_missing && !src.is_missing && no_of_item.value > 0 && no_of_item.value*dest.element_size <= src.length*src.element_size && no_of_item.value <= dest.capacity){
            memcpy((void*)dest.value, (void*)src.value, no_of_item.value*dest.element_size);
        }
        return code;
    }
    _L_OP_MEMMOVE_PTR_PTR_I64:{
        const auto dest = frame_buffer[(++code)->value];
        const auto src = frame_buffer[(++code)->value];
        const auto no_of_item = frame_buffer[(++code)->value];
        if(!dest.is_missing && !no_of_item.is_missing && !src.is_missing && no_of_item.value > 0 && no_of_item.value*dest.element_size <= src.length*src.element_size && no_of_item.value <= dest.capacity){
            memmove((void*)dest.value, (void*)src.value, no_of_item.value*dest.element_size);
        }
        return code;
    }
    _L_OP_I64_UNSIGNED_CMP_PTR_PTR_I64:{
        const auto dest = frame_buffer[(++code)->value];
        const auto src = frame_buffer[(++code)->value];
        const auto no_of_item = frame_buffer[(++code)->value];
        const auto result = frame_buffer + (++code)->value;
        result->type = ValueType::VT_I64;
        result->is_missing = dest.is_missing || src.is_missing || no_of_item.is_missing || no_of_item.value <= 0 || no_of_item.value*dest.element_size > src.length*src.element_size || no_of_item.value > dest.capacity;
        if(result->is_missing){
            result->value = 0;
        }
        else{
            result->value = (std::int64_t)memcmp((void*)dest.value, (void*)src.value, no_of_item.value*dest.element_size);
        }
        return code;
    }
    /*----Time series related--------------------------------------*/
    OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_GE_PTR_I64_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == size.value); 
    OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_LE_PTR_I64_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == -1); 
    OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_GE_PTR_II64_I64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], frame_buffer[(++code)->value],  first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == size.value);
    OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_LE_PTR_II64_I64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == -1);

    OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == size); 
    OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == -1); 
    OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == size);
    OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == -1);

    /*-------Traceback--------------------------------*/
    _L_OP_PRINT_PTR:{
        const auto ptr = frame_buffer[(++code)->value];
        if(!ptr.is_missing && ptr.value != 0){
            std::cout.write((const char*)ptr.value, ptr.length);
            std::cout.flush();
        }
        return code;
    }
}
//path is the array of pc values where we want to trace the execution via OP_TRACE. It is used for debugging
VM_NOCLONE static std::int64_t __attribute__((noinline, used, hot)) execute_vm(std::int64_t frame_size, Value* code, Value* frame_buffer, std::int64_t* traceback_func_loc_idx,
                                                                                      std::int64_t* ret_loc_idx, std::int64_t* func_stack_off, std::vector<std::int64_t>& path) noexcept{
    if(code == nullptr){
        //The first call(from same thread as the original call) to this function will have code = nullptr. So we need to initialize the hot_dispatch table
        /* ---- Loads ----------------------------------------- */
        INSERT(OP_PTR_LOAD_PTR);
        INSERT(OP_I64_LOAD_I64); INSERT(OP_I64_LOAD_FLOAT);
        INSERT(OP_FLOAT_LOAD_I64); INSERT(OP_FLOAT_LOAD_FLOAT);

        INSERT(OP_PTR_LOAD_PTRI);
        INSERT(OP_I64_LOAD_II64); INSERT(OP_FLOAT_LOAD_FLOATI);

        /* ---- Binary Arithmetic Instructions ----------------------------------*/
        INSERT(OP_I64FLOAT_ADD_I64FLOAT_I64FLOAT);
        INSERT(OP_I64FLOAT_SUB_I64FLOAT_I64FLOAT);
        INSERT(OP_I64_MUL_I64_I64); INSERT(OP_FLOAT_MUL_ROUND_FLOAT_FLOAT);
        // INSERT(OP_FLOAT_MUL_TRUNC_FLOAT_FLOAT); 
        INSERT(OP_I64_DIV_I64_I64); INSERT(OP_FLOAT_DIV_FLOAT_FLOAT);
        INSERT(OP_I64FLOAT_REM_I64FLOAT_I64FLOAT);
        INSERT(OP_I64FLOAT_MIN_I64FLOAT_I64FLOAT);
        INSERT(OP_I64FLOAT_MAX_I64FLOAT_I64FLOAT);

        INSERT(OP_I64FLOAT_ADD_I64FLOAT_I64FLOATI);
        INSERT(OP_I64_MUL_I64_II64);INSERT(OP_FLOAT_MUL_ROUND_FLOAT_FLOATI);
        // INSERT(OP_FLOAT_MUL_TRUNC_FLOAT_FLOATI); 
        INSERT(OP_I64_DIV_I64_II64); INSERT(OP_FLOAT_DIV_FLOAT_FLOATI);
        INSERT(OP_I64FLOAT_REM_I64FLOAT_I64FLOATI);
        INSERT(OP_I64FLOAT_MAX_I64FLOAT_I64FLOATI);
        INSERT(OP_I64FLOAT_MIN_I64FLOAT_I64FLOATI);

        INSERT(OP_I64FLOAT_SUB_I64FLOATI_I64FLOAT);
        INSERT(OP_I64_DIV_II64_I64); INSERT(OP_FLOAT_DIV_FLOATI_FLOAT);
        INSERT(OP_I64FLOAT_REM_I64FLOATI_I64FLOAT);

        /* ---- Other Arithmetic Instructions ------- */
        INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_I64_REG_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_REG_WEIGHT); 
        INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_II64_REG_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_REG_WEIGHT); 
        INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_I64_IMM_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_IMM_WEIGHT); 
        INSERT_COLD(OP_I64_WEIGHTED_ADD_II64_II64_IMM_WEIGHT); INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_IMM_WEIGHT);
        // INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_REG_WEIGHT); 
        // INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_REG_WEIGHT); 
        // INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_IMM_WEIGHT);
        // INSERT_COLD(OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_IMM_WEIGHT);

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
        INSERT_COLD(OP_I64_IN_RANGE_PTR_PTR_PTR_I64);
        INSERT_COLD(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64);

        INSERT(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOATI);
        INSERT(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOATI);
        INSERT(OP_I64_GT_PTRI64FLOAT_PTRI64FLOATI);
        INSERT(OP_I64_GE_PTRI64FLOAT_PTRI64FLOATI);
        INSERT(OP_I64_GT_PTRI64FLOATI_PTRI64FLOAT);
        INSERT(OP_I64_GE_PTRI64FLOATI_PTRI64FLOAT);
        INSERT(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
        INSERT(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
        INSERT_COLD(OP_I64_IN_RANGE_PTR_PTR_PTR_II64);
        INSERT_COLD(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_II64);

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
        INSERT_COLD(OP_BR_IN_RANGE_PTR_PTR_PTR_I64);
        
        INSERT(OP_BR_EQ_I64FLOAT_I64FLOATI);
        INSERT(OP_BR_GT_I64FLOAT_I64FLOATI);
        INSERT(OP_BR_GT_I64FLOATI_I64FLOAT);
        INSERT(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
        INSERT_COLD(OP_BR_IN_RANGE_PTR_PTR_PTR_II64);
    
        /*--------Regular branch-----*/
        INSERT(OP_BR);
        INSERT(OP_BR_TRUE);

        /* ---- Calls --------------------------------------- */
        INSERT(OP_CALL);
        INSERT(OP_EXTERN_CALL_PTR);
        INSERT(OP_RET);
    
        /* ---- Heap objects: arrays ---------------------------------------------*/
        //The only reason get and set are not in cold but rest are in cold because I expect get and set that take in immediate values to be used more often due to them being used for structured data. Or else would have sent them to cold also
        //The other use of ptr is time series. We expect them to be used with external function call. I.e not much pressure on the vm bytecode
        INSERT_COLD(OP_PTR_ALLOC_I64_I64_II64);
        INSERT_COLD(OP_PTR_REALLOC_PTR_I64_I64); 
        INSERT_COLD(OP_FREE_PTR); 

        INSERT_COLD(OP_I64_GET_PTR_I64);
        INSERT_COLD(OP_FLOAT_GET_PTR_I64);
        INSERT_COLD(OP_PTR_GET_PTR_I64);

        INSERT(OP_I64_GET_PTR_II64);
        INSERT(OP_FLOAT_GET_PTR_II64);
        INSERT(OP_PTR_GET_PTR_II64);

        INSERT_COLD(OP_PTR_PTROFFSET_ADD_PTR_I64);
        INSERT_COLD(OP_PTR_PTROFFSET_ADD_PTR_II64);
        INSERT_COLD(OP_PTR_PTROFFSET_SUB_PTR_I64);

        INSERT_COLD(OP_SET_PTR_I64_I64FLOAT);
        INSERT_COLD(OP_SET_PTR_I64_PTR);

        INSERT(OP_SET_PTR_II64_I64FLOAT);
        INSERT(OP_SET_PTR_II64_PTR);

        INSERT_COLD(OP_I64_LEN_PTR);
        INSERT_COLD(OP_I64_CAP_PTR);
        INSERT_COLD(OP_I64_ELM_SIZE_PTR);
        INSERT_COLD(OP_POP_PTR_I64);
        INSERT_COLD(OP_PUSH_PTR_PTR_I64_II64);
        INSERT_COLD(OP_PUSH_PTR_I64FLOAT_II64);
        INSERT_COLD(OP_PUSH_PTR_PTR_VALUE_II64);
        INSERT_COLD(OP_MEMCPY_PTR_PTR_I64);
        INSERT_COLD(OP_MEMMOVE_PTR_PTR_I64);
        INSERT_COLD(OP_I64_UNSIGNED_CMP_PTR_PTR_I64);
        
        /*----Time series related--------------------------------------*/
        INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_I64_I64_I64FLOAT); 
        INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_I64_I64_I64FLOAT);  
        INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_II64_I64_I64FLOAT);
        INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_II64_I64_I64FLOAT);

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
        INSERT(OP_IF_TRUE_TRACE);
        INSERT(OP_IF_MISSING_TRACE);

        return -2;
    }
    Value* code_start = code;
    std::int64_t remaining_frame_size = frame_size;
    Value* extern_ret_vals[16];//U can have a most 16 ret values form an external function
    Value extern_args[128];//U can have a most 128 args to an external function
    
    goto *(void*)code[0].value;

    /* ---- Loads ----------------------------------------- */
    OP_SIMPLE_LOAD_INST(OP_PTR_LOAD_PTR, frame_buffer[(++code)->value]);
    OP_SIMPLE_LOAD_INST(OP_I64_LOAD_I64, frame_buffer[(++code)->value]);
    OP_SIMPLE_LOAD_INST(OP_FLOAT_LOAD_FLOAT, frame_buffer[(++code)->value]);
    _L_OP_I64_LOAD_FLOAT:{
        const auto value = frame_buffer[(++code)->value];
        const auto ptr = frame_buffer + (++code)->value;
        ptr->value = value.value/FIXED_POINT_FLOAT_SCALING_FACTOR;\
        ptr->type = ValueType::VT_I64;
        ptr->is_missing = value.is_missing;
        DISPATCH();\
    }
    _L_OP_FLOAT_LOAD_I64:{
        const auto value = frame_buffer[(++code)->value];
        const auto ptr = frame_buffer + (++code)->value;
        ptr->value = value.value*FIXED_POINT_FLOAT_SCALING_FACTOR;\
        ptr->type = ValueType::VT_FLOAT;
        ptr->is_missing = value.is_missing;
        DISPATCH();\
    }

    OP_SIMPLE_LOAD_INST(OP_PTR_LOAD_PTRI, *(++code));
    OP_SIMPLE_LOAD_INST(OP_I64_LOAD_II64, *(++code));
    OP_SIMPLE_LOAD_INST(OP_FLOAT_LOAD_FLOATI, *(++code));

    /* ---- Binary Arithmetic Instructions ----------------------------------*/
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_ADD_I64FLOAT_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value + rhs.value, lhs.is_missing || rhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_SUB_I64FLOAT_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value - rhs.value, lhs.is_missing || rhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64_MUL_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value * rhs.value, lhs.is_missing || rhs.is_missing, lhs.type);
    // OP_SIMPLE_BINARY_INST(OP_FLOAT_MUL_TRUNC_FLOAT_FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
    //                       static_cast<int64_t>((static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs.value)) / FIXED_POINT_FLOAT_SCALING_FACTOR));
    _L_OP_FLOAT_MUL_ROUND_FLOAT_FLOAT:{
        const auto lhs = frame_buffer[(++code)->value];
        const auto rhs = frame_buffer[(++code)->value];
        const auto ptr = frame_buffer + (++code)->value;
        __int128 product = static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs.value);
        __int128 result = (product >= 0)
                            ? (product + FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR
                            : (product - FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR;
        ptr->value = static_cast<int64_t>(result);
        ptr->type = lhs.type;
        ptr->is_missing = lhs.is_missing || rhs.is_missing;
        DISPATCH();
    }
    OP_SIMPLE_DIV_REM_INST(OP_I64_DIV_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value / rhs.value, lhs.is_missing || rhs.is_missing || rhs.value == 0 || (lhs.value == INT64_MIN && rhs.value == -1), lhs.type);
    OP_SIMPLE_FLOAT_DIV_INST(OP_FLOAT_DIV_FLOAT_FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                             static_cast<int64_t>((static_cast<__int128>(lhs.value) * FIXED_POINT_FLOAT_SCALING_FACTOR) / static_cast<__int128>(rhs.value)),
                             lhs.is_missing || rhs.is_missing || rhs.value == 0, lhs.type);
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOAT_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value % rhs.value, lhs.is_missing || rhs.is_missing || rhs.value == 0 || (lhs.value == INT64_MIN && rhs.value == -1), lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MIN_I64FLOAT_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], std::min(lhs.value, rhs.value), lhs.is_missing || rhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MAX_I64FLOAT_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], std::max(lhs.value, rhs.value), lhs.is_missing || rhs.is_missing, lhs.type);

    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_ADD_I64FLOAT_I64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value + rhs, lhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64_MUL_I64_II64, frame_buffer[(++code)->value], (++code)->value, lhs.value * rhs, lhs.is_missing, lhs.type);
    // OP_SIMPLE_BINARY_INST(OP_FLOAT_MUL_TRUNC_FLOAT_FLOATI, frame_buffer[(++code)->value], *(++code), 
    //                       static_cast<int64_t>((static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs.value)) / FIXED_POINT_FLOAT_SCALING_FACTOR));
    _L_OP_FLOAT_MUL_ROUND_FLOAT_FLOATI:{
        const auto lhs = frame_buffer[(++code)->value];
        const auto rhs = (++code)->value;
        const auto ptr = frame_buffer + (++code)->value;
        __int128 product = static_cast<__int128>(lhs.value) * static_cast<__int128>(rhs);
        __int128 result = (product >= 0)
                            ? (product + FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR
                            : (product - FIXED_POINT_FLOAT_SCALING_FACTOR / 2) / FIXED_POINT_FLOAT_SCALING_FACTOR;
        ptr->value = static_cast<int64_t>(result);
        ptr->type = lhs.type;
        ptr->is_missing = lhs.is_missing;
        DISPATCH();
    }
    OP_SIMPLE_DIV_REM_INST(OP_I64_DIV_I64_II64, frame_buffer[(++code)->value], (++code)->value, lhs.value / rhs, lhs.is_missing || (lhs.value == INT64_MIN && rhs == -1), lhs.type);
    OP_SIMPLE_FLOAT_DIV_INST(OP_FLOAT_DIV_FLOAT_FLOATI, frame_buffer[(++code)->value], (++code)->value, 
                             static_cast<int64_t>((static_cast<__int128>(lhs.value) * FIXED_POINT_FLOAT_SCALING_FACTOR) / static_cast<__int128>(rhs)),
                             lhs.is_missing , lhs.type);
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOAT_I64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value % rhs, lhs.is_missing || (lhs.value == INT64_MIN && rhs == -1), lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MIN_I64FLOAT_I64FLOATI, frame_buffer[(++code)->value], (++code)->value, std::min(lhs.value, rhs), lhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_MAX_I64FLOAT_I64FLOATI, frame_buffer[(++code)->value], (++code)->value, std::max(lhs.value, rhs), lhs.is_missing, lhs.type);

    OP_SIMPLE_BINARY_INST(OP_I64FLOAT_SUB_I64FLOATI_I64FLOAT, (++code)->value, frame_buffer[(++code)->value], lhs - rhs.value, rhs.is_missing, rhs.type);
    OP_SIMPLE_DIV_REM_INST(OP_I64_DIV_II64_I64, (++code)->value, frame_buffer[(++code)->value], lhs / rhs.value, rhs.is_missing || rhs.value == 0 || (lhs == INT64_MIN && rhs.value == -1), rhs.type);
    OP_SIMPLE_FLOAT_DIV_INST(OP_FLOAT_DIV_FLOATI_FLOAT, (++code)->value, frame_buffer[(++code)->value], 
                             static_cast<int64_t>((static_cast<__int128>(lhs) * FIXED_POINT_FLOAT_SCALING_FACTOR) / static_cast<__int128>(rhs.value)),
                             rhs.is_missing || rhs.value == 0, rhs.type);
    OP_SIMPLE_DIV_REM_INST(OP_I64FLOAT_REM_I64FLOATI_I64FLOAT, (++code)->value, frame_buffer[(++code)->value], lhs % rhs.value, rhs.is_missing || rhs.value == 0 || (lhs == INT64_MIN && rhs.value == -1), rhs.type);

    /* ---- Binary Bitwise Instructions ----------- */
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_AND_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value & rhs.value, lhs.is_missing || rhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_OR_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value | rhs.value, lhs.is_missing || rhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_XOR_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value ^ rhs.value, lhs.is_missing || rhs.is_missing, lhs.type);
    OP_SIMPLE_BINARY_INST(OP_I64_BIT_XOR_I64_II64, frame_buffer[(++code)->value], (++code)->value, lhs.value ^ rhs, lhs.is_missing, lhs.type);

    /* ---- Comparison Instruction ------------------*/
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value == rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value != rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GT_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value > rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GE_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value >= rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_IN_RANGE_INST(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT, 
                            frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                            frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                            is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value),
                            arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4.is_missing);
    OP_SIMPLE_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT, 
                            frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                            frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                            !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value),
                            arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4.is_missing);
    
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value == rhs, lhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value != rhs, lhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GT_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value > rhs, lhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GE_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value >= rhs, lhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GT_PTRI64FLOATI_PTRI64FLOAT, (++code)->value, frame_buffer[(++code)->value], lhs > rhs.value, rhs.is_missing);
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_GE_PTRI64FLOATI_PTRI64FLOAT, (++code)->value, frame_buffer[(++code)->value], lhs >= rhs.value, rhs.is_missing);
    OP_SIMPLE_IN_RANGE_INST(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI, 
                            frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                            frame_buffer[(++code)->value], (++code)->value, 
                            is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4),
                            arg1.is_missing || arg2.is_missing || arg3.is_missing);
    OP_SIMPLE_IN_RANGE_INST(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI, 
                            frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                            frame_buffer[(++code)->value], (++code)->value, 
                            !is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4),
                            arg1.is_missing || arg2.is_missing || arg3.is_missing);
    
    /* ---- Unary Instruction ------------------*/
    OP_SIMPLE_UNARY_INST(OP_I64_HAS_VALUE, frame_buffer[(++code)->value], ValueType::VT_I64, arg.is_missing, false);
    OP_SIMPLE_UNARY_INST(OP_I64_HAS_NO_VALUE, frame_buffer[(++code)->value], ValueType::VT_I64, arg.is_missing, false);

    /* ---- Fused Compare-and-Branch(Takes in 2 or 4 operands + 3 branches(if branch,else branch and data missing branch)) ----------------------------------------*/
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_EQ_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value == rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_GT_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value > rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_OR_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value || rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_AND_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value && rhs.value, lhs.is_missing || rhs.is_missing);
    OP_SIMPLE_BR_IN_RANGE_INST(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT, 
                               frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                               frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                               is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4.value),
                               arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4.is_missing);
    
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_EQ_I64FLOAT_I64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value == rhs, lhs.is_missing);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_GT_I64FLOAT_I64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value > rhs, lhs.is_missing);
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_GT_I64FLOATI_I64FLOAT, (++code)->value, frame_buffer[(++code)->value], lhs > rhs.value, rhs.is_missing);
    OP_SIMPLE_BR_IN_RANGE_INST(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI, 
                               frame_buffer[(++code)->value], frame_buffer[(++code)->value], 
                               frame_buffer[(++code)->value], (++code)->value, 
                               is_in_range_with_step(arg1.value, arg2.value, arg3.value, arg4),
                               arg1.is_missing || arg2.is_missing || arg3.is_missing);

    /*--------Regular branch-----*/
    _L_OP_BR:{
        GOTO((Value*)((++code)->value));
    }
    _L_OP_BR_TRUE:{
        const auto condition = frame_buffer[(++code)->value];
        const auto label1 = (Value*)((++code)->value);
        const auto label2 = (Value*)((++code)->value);
        const auto label3 = (Value*)((++code)->value);
        if(condition.is_missing){GOTO(label3);}
        else if(condition.value){GOTO(label1);}
        else{GOTO(label2);}
    }

    /* ---- Calls --------------------------------------- */
    _L_OP_CALL:{
        const auto func_loc = (Value*)((++code)->value);
        const auto stack_off = (++code)->value;//IT is 0 if it is a tail call and you want to reuse the current stack frame
        const auto num_ret = (++code)->value;
        const auto num_args = (++code)->value;
        remaining_frame_size -= stack_off;
        frame_buffer[stack_off + num_ret] = Value((std::int64_t)(frame_buffer+stack_off), remaining_frame_size, remaining_frame_size, sizeof(Value));//Contains the stack frame
        for(std::int64_t i = 0; i < num_args; i++){
            frame_buffer[stack_off + 1 + num_ret + i] = frame_buffer[(++code)->value];
        }
        frame_buffer += stack_off;
        *traceback_func_loc_idx = func_loc - code_start;
        *func_stack_off = stack_off;
        *ret_loc_idx = code - code_start;

        traceback_func_loc_idx++;
        func_stack_off++;
        ret_loc_idx++;

        GOTO(func_loc);
    }
    _L_OP_EXTERN_CALL_PTR:{
        const auto func_ptr = (ExternFuncType)(++code)->value;
        const auto num_args = (++code)->value;
        const auto num_ret = (++code)->value;
        // Value** ret_vals = (Value**)malloc(sizeof(Value*) * num_ret);
        for(std::int64_t i = 0; i < num_ret; i++){
            extern_ret_vals[i] = frame_buffer + (++code)->value;
        }
        for(std::int64_t i = 0; i < num_args; i++){
            extern_args[i] = frame_buffer[(++code)->value];
        }
        memset(extern_ret_vals + num_ret, 0, sizeof(Value*) * (16 - num_ret));//Reset the rest of the ret vals to nullptr
        memset(extern_args + num_args, 0, sizeof(Value) * (128 - num_args));//Reset the rest of the args to 0
        func_ptr(extern_args, extern_ret_vals);
        DISPATCH();
    }
    _L_OP_RET:{
        const auto num_ret = (++code)->value;
        for(std::int64_t i = 0; i < num_ret; i++){
            frame_buffer[i] = frame_buffer[(++code)->value];
        }
        traceback_func_loc_idx--;
        func_stack_off--;
        ret_loc_idx--;
        remaining_frame_size += *func_stack_off;
        frame_buffer -= *func_stack_off;
        code = code_start + *ret_loc_idx;
        *traceback_func_loc_idx = -1;
        *func_stack_off = -1;
        *ret_loc_idx = -1;
        DISPATCH();
    }
    /* ---- Heap objects: arrays ---------------------------------------------*/
    OP_SIMPLE_GET_INT64FLOAT_INST(OP_I64_GET_PTR_II64, frame_buffer[(++code)->value], ValueType::VT_I64);
    OP_SIMPLE_GET_INT64FLOAT_INST(OP_FLOAT_GET_PTR_II64, frame_buffer[(++code)->value], ValueType::VT_I64);
    OP_SIMPLE_GET_PTR_INST(OP_PTR_GET_PTR_II64, frame_buffer[(++code)->value]);

    OP_SIMPLE_SET_INT64FLOAT_INST(OP_SET_PTR_II64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value]);
    OP_SIMPLE_SET_PTR_INST(OP_SET_PTR_II64_PTR, frame_buffer[(++code)->value], frame_buffer[(++code)->value]);

    /* ---- No-ops / control -------------------------------------------- */
    _L_OP_NOP:{
        DISPATCH();
    }
    _L_OP_HALT:{
        return -1;
    }
    _L_OP_TRAP:{
        return code - code_start;
    }
    _L_OP_IF_TRUE_TRAP:{
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(condition.is_missing || !condition.value){
            return _pc;
        }
        else{DISPATCH();}
    }
    _L_OP_IF_MISSING_TRAP:{
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(!condition.is_missing){
            return _pc;
        }
        else{DISPATCH();}
    }
    _L_OP_TRACE:{
        path.push_back(code - code_start);
        DISPATCH();
    }
    _L_OP_IF_TRUE_TRACE:{
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(condition.is_missing || !condition.value){DISPATCH();}
        else{path.push_back(_pc); DISPATCH();}
    }
    _L_OP_IF_MISSING_TRACE:{
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(!condition.is_missing){DISPATCH();}
        else{path.push_back(_pc); DISPATCH();}
    }
    ___L_COLD_LABEL:{
        code = execute_cold_inst(code, frame_buffer);//code on the last argument of the instruction or on the instruction if it has no argument
        DISPATCH();
    }
}


void set_dispatch_table(){
    execute_cold_inst(nullptr, nullptr);//This will initialize the cold_dispatch table
    std::vector<std::int64_t> dummy_path;
    execute_vm(0, nullptr, nullptr, nullptr, nullptr, nullptr, dummy_path);//This will initialize the hot_dispatch table
}

void write_addr(Value* code, std::int64_t code_size){
    for(std::int64_t i = 0; i < code_size; i++){
        if(code[i].type == ValueType::VT_INSTRUCTION){
            const auto inst = code[i].value;
            code[i].value = (std::int64_t)hot_dispatch[inst];
            code[i].capacity = (std::int64_t)cold_dispatch[inst];
        }
        else if(code[i].type == ValueType::VT_LABEL_LOC){
            const auto label = code[i].value;
            code[i].value = (std::int64_t)(code + label);
        }
        else if(code[i].type == ValueType::VT_FUNC_LOC){
            const auto func_loc = code[i].value;
            code[i].value = (std::int64_t)(code + func_loc);
        }
    }
}
}
}