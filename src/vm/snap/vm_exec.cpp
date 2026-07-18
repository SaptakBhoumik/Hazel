//TODO:Mark every label/branch with [[likely]] or [[unlikely]] 
#include "vm/value.hpp"
#include "vm/vm.hpp"
#include "macros.hpp"
#include <cstddef>
#include <cstdint>
#include <cmath>
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
namespace VM{
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
        COLD_INSERT(OP_PTR_SLICE_PTR_I64);
        COLD_INSERT(OP_PTR_PTROFFSET_ADD_PTR_I64);
        COLD_INSERT(OP_PTR_PTROFFSET_ADD_PTR_II64);
        COLD_INSERT(OP_PTR_PTROFFSET_SUB_PTR_I64);
        COLD_INSERT(OP_SET_PTR_I64_I64FLOAT);
        COLD_INSERT(OP_SET_PTR_I64_PTR);
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
        // COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_I64_I64_I64FLOAT); 
        // COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_I64_I64_I64FLOAT);  
        // COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_II64_I64_I64FLOAT);
        // COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_II64_I64_I64FLOAT);

        // COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT); 
        // COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT);  
        // COLD_INSERT(OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT);
        // COLD_INSERT(OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT);

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

    _L_OP_PTR_SLICE_PTR_I64:{
        const auto ptr = frame_buffer + (++code)->value;
        const auto num_count = frame_buffer[(++code)->value];
        const auto dest_ptr = frame_buffer + (++code)->value;
        dest_ptr->type = ValueType::VT_PTR;
        dest_ptr->is_missing = ptr->is_missing || num_count.is_missing || num_count.value < 0 || num_count.value > ptr->capacity;
        if(!dest_ptr->is_missing){
            dest_ptr->value = ptr->value;
            dest_ptr->length = std::min(num_count.value, ptr->length);
            dest_ptr->capacity = num_count.value;
            dest_ptr->element_size = ptr->element_size;
            ptr->value += num_count.value*ptr->element_size;
            ptr->length = std::max<std::int64_t>(ptr->length - num_count.value, 0);
            ptr->capacity = std::max<std::int64_t>(ptr->capacity - num_count.value, 0);
        }
        DISPATCH();
    }

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
    // /*----Time series related--------------------------------------*/
    // OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_GE_PTR_I64_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == size.value); 
    // OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_LE_PTR_I64_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == -1); 
    // OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_GE_PTR_II64_I64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], frame_buffer[(++code)->value],  first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == size.value);
    // OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(OP_I64_BINARY_FIND_LE_PTR_II64_I64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size.value, value.value), result_ptr->value == -1);

    // OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == size); 
    // OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == -1); 
    // OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], first_ge((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == size);
    // OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT, frame_buffer[(++code)->value], *(++code), frame_buffer[(++code)->value], first_le((std::int64_t*)(ptr.value+start_idx.value*ptr.element_size), size, value.value), result_ptr->value == -1);

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

struct Context{
    Value** frame_buffers;
    Value** old_frame;//old_frame[idx] = frame_buffers[idx] + some offset
    std::int64_t idx;//Index of current frame buffer
    std::int64_t size;//Total number of frame buffers(Not all may be allocated)
    std::int64_t allocated_size;//Total number of frame buffers allocated
    std::int64_t first_frame_size;//Size of the first frame buffer. The most important frame buffer and usually larger
    std::int64_t frame_size;//Size of each frame buffer
    std::int64_t max_no_of_allocated_unused_frames;//Maximum number of unused frames that can be allocated. This is used to prevent memory exhaustion

    struct CallFrame{
        std::int64_t traceback_func_loc_idx;
        std::int64_t func_stack_off;
        std::int64_t ret_loc_idx;
        Value* ret_loc_start;//Location from where we start putting return values
    };
    CallFrame* call_frame;
    std::int64_t curr_call_depth;
    std::int64_t max_possible_func_call_depth;//It can grow. Basically the capacity of stuff like traceback_func_loc_idx, ret_loc_idx, func_stack_off, ret_loc_start. The capacity can grow if needed. Not a hard limit
    std::int64_t func_call_depth_ceil;//Negative if no limit

    std::int64_t* path;
    std::int64_t path_cap;
    std::int64_t path_idx;
};

struct ReturnValue{
    std::int64_t pc;
    enum class ErrorCode:std::int8_t{
        INIT_DISPATCH_TABLE,
        
        EXTERN_FUNC_HALT,
        EXTERN_FUNC_TRAP,
        
        CALL_DEPTH_EXCEEDED,
        STACK_FRAME_ALLOC_FAILED,

        REGULAR_HALT,
        REGULAR_TRAP,
        CONDITIONAL_TRAP,
        MISSING_VALUE_TRAP,
    };
    ErrorCode err;
    
    ReturnValue(std::int64_t pc, ErrorCode err):pc(pc),err(err){}
};
//path is the array of pc values where we want to trace the execution via OP_TRACE. It is used for debugging
VM_NOCLONE static ReturnValue __attribute__((noinline, used, hot)) execute_vm(Value* code, Context* context, Value** extern_ret_vals, Value** extern_args) noexcept{
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
        INSERT(OP_I64_NE_PTRI64FLOAT_PTRI64FLOAT);
        INSERT(OP_I64_GT_PTRI64FLOAT_PTRI64FLOAT);
        INSERT(OP_I64_GE_PTRI64FLOAT_PTRI64FLOAT);
        INSERT(OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
        INSERT(OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
        INSERT_COLD(OP_I64_IN_RANGE_PTR_PTR_PTR_I64);
        INSERT_COLD(OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64);

        INSERT(OP_I64_EQ_PTRI64FLOAT_PTRI64FLOATI);
        INSERT(OP_I64_NE_PTRI64FLOAT_PTRI64FLOATI);
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
        INSERT(OP_BR_XOR_I64_I64);
        INSERT(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT);
        INSERT_COLD(OP_BR_IN_RANGE_PTR_PTR_PTR_I64);
        
        INSERT(OP_BR_EQ_I64FLOAT_I64FLOATI);
        INSERT(OP_BR_GT_I64FLOAT_I64FLOATI);
        INSERT(OP_BR_GT_I64FLOATI_I64FLOAT);
        INSERT(OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI);
        INSERT_COLD(OP_BR_IN_RANGE_PTR_PTR_PTR_II64);
    
        /*--------Regular branch-----*/
        INSERT(OP_BR);

        /* ---- Calls --------------------------------------- */
        INSERT(OP_EXTERN_CALL_PTR);
        INSERT(OP_CALL);
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

        INSERT_COLD(OP_PTR_SLICE_PTR_I64);
        INSERT(OP_PTR_SLICE_PTR_II64);//Much more common.

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
        // INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_I64_I64_I64FLOAT); 
        // INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_I64_I64_I64FLOAT);  
        // INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_II64_I64_I64FLOAT);
        // INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_II64_I64_I64FLOAT);

        // INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT); 
        // INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT);  
        // INSERT_COLD(OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT);
        // INSERT_COLD(OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT);

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

        return ReturnValue(0, ReturnValue::ErrorCode::INIT_DISPATCH_TABLE);
    }
    Value* code_start = code;
    Value* frame_buffer = context->frame_buffers[context->idx];
    std::int64_t remaining_frame_size = context->first_frame_size;
    
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
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_NE_PTRI64FLOAT_PTRI64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value != rhs.value, lhs.is_missing || rhs.is_missing);
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
    OP_SIMPLE_CMP_BINARY_INST(OP_I64_NE_PTRI64FLOAT_PTRI64FLOATI, frame_buffer[(++code)->value], (++code)->value, lhs.value != rhs, lhs.is_missing);
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
    OP_SIMPLE_BR_BINARY_CMP_INST(OP_BR_XOR_I64_I64, frame_buffer[(++code)->value], frame_buffer[(++code)->value], lhs.value ^ rhs.value, lhs.is_missing || rhs.is_missing);
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

    /* ---- Calls --------------------------------------- */
    //TODO:Extern call and call and ret may only need to support one return(Depends on how the IR memory layout is designed). SO update it afterwards accordingly
    _L_OP_EXTERN_CALL_PTR:{
        const auto _pc = code - code_start;
        const auto func_ptr = (ExternFuncType)(++code)->value;
        const auto num_args = (++code)->value;
        const auto num_ret = (++code)->value;
        for(std::int64_t i = 0; i < num_ret; i++){
            extern_ret_vals[i] = frame_buffer + (++code)->value;
        }
        for(std::int64_t i = 0; i < num_args; i++){
            extern_args[i] = frame_buffer + (++code)->value;//These external function are provided by the compiler so we can trust them to not do weird stuff if pass by ptr
        }
        auto error_code = func_ptr(extern_args, extern_ret_vals);
        if(error_code == -1){
            return ReturnValue(_pc, ReturnValue::ErrorCode::EXTERN_FUNC_HALT);
        }
        else if(error_code == -2){
            return ReturnValue(_pc, ReturnValue::ErrorCode::EXTERN_FUNC_TRAP);
        }
        else if(error_code == -3){
            context->path[context->path_idx] = _pc;
            context->path_idx++;
            if(context->path_idx >= context->path_cap){
                context->path_cap *= 2;
                context->path = (std::int64_t*)realloc(context->path, sizeof(std::int64_t) * context->path_cap);
            }
            // path.push_back(_pc);//A trace but not trap
        }
        DISPATCH();//Either from a trace or from success(error_code == 0).
    }
    _L_OP_CALL:{
        const auto _pc = code - code_start;
        if(context->func_call_depth_ceil >= 0 && context->curr_call_depth >= context->func_call_depth_ceil) [[unlikely]]{
            return ReturnValue(_pc, ReturnValue::ErrorCode::CALL_DEPTH_EXCEEDED);
        }
        const auto func_loc = (Value*)((++code)->value);
        auto stack_off = (++code)->value;//It can be zero if it is tail call but that assumes there is no overlap in frame_buffer[_stack_off + 1 + i] = caller_frame_buffer[(++code)->value];
                                                       //This is very easy to verify at the compiler level
        auto _stack_off = stack_off;
        const auto needed_stack_size = (++code)->value;//Assumed to be less than context->frame_size
        const auto num_args = (++code)->value;
        const auto ret_loc_start = frame_buffer + (++code)->value;
        const auto caller_frame_buffer = frame_buffer;  

        remaining_frame_size -= stack_off;
        if(remaining_frame_size < needed_stack_size) [[unlikely]]{
            context->old_frame[context->idx] = frame_buffer;
            context->idx++;
            if(context->idx >= context->size) [[unlikely]]{
                context->size *= 2;
                context->frame_buffers = (Value**)realloc(context->frame_buffers, sizeof(Value*) * context->size);
                context->old_frame = (Value**)realloc(context->old_frame, sizeof(Value*) * context->size);
                if(context->frame_buffers == nullptr || context->old_frame == nullptr) [[unlikely]] {
                    return ReturnValue(_pc, ReturnValue::ErrorCode::STACK_FRAME_ALLOC_FAILED);
                }
            }
            if(context->idx >= context->allocated_size){
                context->allocated_size++;
                context->frame_buffers[context->idx] = (Value*)malloc(sizeof(Value) * context->frame_size);
                if(context->frame_buffers[context->idx] == nullptr) [[unlikely]] {
                    return ReturnValue(_pc, ReturnValue::ErrorCode::STACK_FRAME_ALLOC_FAILED);
                }
            }
            frame_buffer = context->frame_buffers[context->idx];
            remaining_frame_size = context->frame_size;
            _stack_off = 0;
            stack_off = 1;
        }
        frame_buffer[_stack_off] = Value((std::int64_t)(frame_buffer+_stack_off), remaining_frame_size, remaining_frame_size, sizeof(Value));//Contains the stack frame
        for(std::int64_t i = 0; i < num_args; i++){
            frame_buffer[_stack_off + 1 + i] = caller_frame_buffer[(++code)->value];
        }
        frame_buffer += _stack_off;
        
        if(context->curr_call_depth >= context->max_possible_func_call_depth) [[unlikely]]{
            context->max_possible_func_call_depth *= 2;
            context->call_frame = (Context::CallFrame*)realloc(context->call_frame, sizeof(Context::CallFrame) * context->max_possible_func_call_depth);
            if(context->call_frame == nullptr) [[unlikely]]{
                return ReturnValue(_pc, ReturnValue::ErrorCode::STACK_FRAME_ALLOC_FAILED);
            }
        }
        context->call_frame[context->curr_call_depth] = Context::CallFrame{func_loc - code_start, stack_off, 
                                                                           code - code_start, ret_loc_start};
        context->curr_call_depth++;
        GOTO(func_loc);
    }
    _L_OP_RET:{
        const auto num_ret = (++code)->value;
        for(std::int64_t i = 0; i < num_ret; i++){
            *(context->call_frame[context->curr_call_depth-1].ret_loc_start + i) = frame_buffer[(++code)->value];
        }
        context->curr_call_depth--;
        remaining_frame_size += context->call_frame[context->curr_call_depth].func_stack_off;
        frame_buffer -= context->call_frame[context->curr_call_depth].func_stack_off;
        if(remaining_frame_size > context->frame_size && context->idx > 0) [[unlikely]]{//IT cant happen when index is 0
            if(context->allocated_size - context->idx > context->max_no_of_allocated_unused_frames){//After this return the current value of context->idx is the number of frames in use
                free(context->frame_buffers[context->idx]);
                context->allocated_size--;
            }
            context->idx--;
            frame_buffer = context->old_frame[context->idx];
            const auto buf_capacity = (context->idx == 0) ? context->first_frame_size : context->frame_size;
            remaining_frame_size = buf_capacity - (frame_buffer - context->frame_buffers[context->idx]);
        }
        code = code_start + context->call_frame[context->curr_call_depth].ret_loc_idx;
        DISPATCH();
    }
    /* ---- Heap objects: arrays ---------------------------------------------*/
    OP_SIMPLE_GET_INT64FLOAT_INST(OP_I64_GET_PTR_II64, frame_buffer[(++code)->value], ValueType::VT_I64);
    OP_SIMPLE_GET_INT64FLOAT_INST(OP_FLOAT_GET_PTR_II64, frame_buffer[(++code)->value], ValueType::VT_I64);
    OP_SIMPLE_GET_PTR_INST(OP_PTR_GET_PTR_II64, frame_buffer[(++code)->value]);

    _L_OP_PTR_SLICE_PTR_II64:{
        const auto ptr = frame_buffer + (++code)->value;
        const auto num_count = (++code)->value;
        const auto dest_ptr = frame_buffer + (++code)->value;
        dest_ptr->type = ValueType::VT_PTR;
        dest_ptr->is_missing = ptr->is_missing || num_count < 0 || num_count > ptr->capacity;
        if(!dest_ptr->is_missing){
            dest_ptr->value = ptr->value;
            dest_ptr->length = std::min(num_count, ptr->length);
            dest_ptr->capacity = num_count;
            dest_ptr->element_size = ptr->element_size;
            ptr->value += num_count*ptr->element_size;
            ptr->length = std::max<std::int64_t>(ptr->length - num_count, 0);
            ptr->capacity = std::max<std::int64_t>(ptr->capacity - num_count, 0);
        }
        DISPATCH();
    }
    
    OP_SIMPLE_SET_INT64FLOAT_INST(OP_SET_PTR_II64_I64FLOAT, frame_buffer[(++code)->value], frame_buffer[(++code)->value]);
    OP_SIMPLE_SET_PTR_INST(OP_SET_PTR_II64_PTR, frame_buffer[(++code)->value], frame_buffer[(++code)->value]);

    /* ---- No-ops / control -------------------------------------------- */
    _L_OP_NOP: [[likely]] {
        DISPATCH();
    }
    _L_OP_HALT: [[unlikely]] {
        return ReturnValue(-1, ReturnValue::ErrorCode::REGULAR_HALT);
    }
    _L_OP_TRAP: [[unlikely]] {
        return ReturnValue(code - code_start, ReturnValue::ErrorCode::REGULAR_TRAP);
    }
    _L_OP_IF_TRUE_TRAP: [[unlikely]] {
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(condition.is_missing || !condition.value){
            return ReturnValue(_pc, ReturnValue::ErrorCode::CONDITIONAL_TRAP);
        }
        else{DISPATCH();}
    }
    _L_OP_IF_MISSING_TRAP: [[unlikely]] {
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(!condition.is_missing){
            return ReturnValue(_pc, ReturnValue::ErrorCode::MISSING_VALUE_TRAP);
        }
        else{DISPATCH();}
    }
    _L_OP_TRACE: [[likely]] {
        context->path[context->path_idx] = code - code_start;
        context->path_idx++;
        if(context->path_idx >= context->path_cap){
            context->path_cap *= 2;
            context->path = (std::int64_t*)realloc(context->path, sizeof(std::int64_t) * context->path_cap);
        }
        DISPATCH();
    }
    _L_OP_IF_TRUE_TRACE: [[likely]] {
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(condition.is_missing || !condition.value){}
        else{
            context->path[context->path_idx] = _pc;
            context->path_idx++;
            if(context->path_idx >= context->path_cap){
                context->path_cap *= 2;
                context->path = (std::int64_t*)realloc(context->path, sizeof(std::int64_t) * context->path_cap);
            }
        }
        DISPATCH();
    }
    _L_OP_IF_MISSING_TRACE: [[likely]] {
        const auto _pc = code - code_start;
        const auto condition = frame_buffer[(++code)->value];
        if(!condition.is_missing){}
        else{
            context->path[context->path_idx] = _pc;
            context->path_idx++;
            if(context->path_idx >= context->path_cap){
                context->path_cap *= 2;
                context->path = (std::int64_t*)realloc(context->path, sizeof(std::int64_t) * context->path_cap);
            }
        }
        DISPATCH();
    }
    ___L_COLD_LABEL: [[unlikely]] {
        code = execute_cold_inst(code, frame_buffer);//code on the last argument of the instruction or on the instruction if it has no argument
        DISPATCH();
    }
}


void set_dispatch_table(){
    execute_cold_inst(nullptr, nullptr);//This will initialize the cold_dispatch table
    execute_vm(nullptr, nullptr, nullptr, nullptr);//This will initialize the hot_dispatch table
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
}