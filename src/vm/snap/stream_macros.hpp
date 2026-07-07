#pragma once

#define FIXED_POINT_FLOAT_SCALING_FACTOR 10000//Because 4 decimal places

#define INSERT(_op) dispatch[(std::uint64_t)Opcode::_op]=&&_L_##_op;
#define INSERT_COLD(_op) dispatch[(std::uint64_t)Opcode::_op]=&&___L_COLD_LABEL;
#define DISPATCH() goto *dispatch[(std::uint64_t)code[++pc].value];
#define GOTO(_loc) pc = _loc; goto *dispatch[(std::uint64_t)code[pc].value];

/* ---- Loads ----------------------------------------- */
#define OP_SIMPLE_LOAD_INST(_op, _value) _L_##_op:{\
                                            const auto value = _value;\
                                            frame_buffer[code[++pc].value] = value;\
                                            DISPATCH();\
                                        }

/* ---- Binary Arithmetic Instructions ----------------------------------*/
#define OP_SIMPLE_BINARY_INST(_op, _lhs, _rhs, _expr) _L_##_op:{\
                                                        const auto lhs = _lhs;\
                                                        const auto rhs = _rhs;\
                                                        const auto ptr = frame_buffer + code[++pc].value;\
                                                        ptr->type = lhs.type;\
                                                        ptr->is_missing = lhs.is_missing || rhs.is_missing;\
                                                        ptr->value = _expr;\
                                                        DISPATCH();\
                                                    }

/* ---- Trinary Arithmetic Instructions ------- */
#define OP_SIMPLE_FMA_INST(_op, _lhs, _rhs, _expr) _L_##_op:{\
                                                        const auto lhs = _lhs;\
                                                        const auto rhs = _rhs;\
                                                        const auto ptr = frame_buffer + code[++pc].value;\
                                                        ptr->type = lhs.type;\
                                                        ptr->is_missing = lhs.is_missing || rhs.is_missing;\
                                                        ptr->value = _expr;\
                                                        DISPATCH();\

#define OP_SIMPLE_DIV_REM_INST(_op, _lhs, _rhs, _expr) _L_##_op:{\
                                                        const auto lhs = _lhs;\
                                                        const auto rhs = _rhs;\
                                                        const auto ptr = frame_buffer + code[++pc].value;\
                                                        ptr->type = lhs.type;\
                                                        ptr->is_missing = lhs.is_missing || rhs.is_missing || rhs.value == 0 || (lhs.value == INT64_MIN && rhs.value == -1);\
                                                        ptr->value = ptr->is_missing ? 0 : _expr;\
                                                        DISPATCH();\
                                                    }

/* ---- Comparison Instruction ------------------*/
#define OP_SIMPLE_CMP_BINARY_INST(_op, _lhs, _rhs, _expr) _L_##_op:{\
                                                            const auto lhs = _lhs;\
                                                            const auto rhs = _rhs;\
                                                            const auto ptr = frame_buffer + code[++pc].value;\
                                                            ptr->type = ValueType::VT_I64;\
                                                            ptr->is_missing = lhs.is_missing || rhs.is_missing;\
                                                            ptr->value = _expr;\
                                                            DISPATCH();\
                                                        }

#define OP_SIMPLE_IN_RANGE_INST(_op, _arg1, _arg2, _arg3, _arg4, _expr) _L_##_op:{\
                                                                        const auto arg1 = _arg1;\
                                                                        const auto arg2 = _arg2;\
                                                                        const auto arg3 = _arg3;\
                                                                        const auto arg4 = _arg4;\
                                                                        const auto ptr = frame_buffer + code[++pc].value;\
                                                                        ptr->type = ValueType::VT_I64;\
                                                                        ptr->is_missing = arg1.is_missing || arg2.is_missing || arg3.is_missing || arg4.is_missing;\
                                                                        ptr->value = _expr;\
                                                                        DISPATCH();\
                                                                    }
/* ---- Unary Instruction ------------------*/
#define OP_SIMPLE_UNARY_INST(_op, _arg, _type, _expr, _is_missing) _L_##_op:{\
                                                                    const auto arg = _arg;\
                                                                    const auto ptr = frame_buffer + code[++pc].value;\
                                                                    ptr->type = _type;\
                                                                    ptr->is_missing = _is_missing;\
                                                                    ptr->value = _expr;\
                                                                    DISPATCH();\
                                                                }
    
#define OP_SIMPLE_COLD_UNARY_INST(_op, _arg, _type, _expr, _is_missing) _L_##_op:{\
                                                                            const auto arg = _arg;\
                                                                            const auto ptr = frame_buffer + code[++pc].value;\
                                                                            ptr->type = _type;\
                                                                            ptr->is_missing = _is_missing;\
                                                                            ptr->value = _expr;\
                                                                            return pc;\
                                                                        }

/* ---- Fused Compare-and-Branch(Takes in 2 or 4 operands + 3 branches(if branch,else branch and data missing branch)) ----------------------------------------*/
#define OP_SIMPLE_BR_BINARY_CMP_INST(_op, _lhs, _rhs, _expr) _L_##_op:{\
                                                                const auto lhs = _lhs;\
                                                                const auto rhs = _rhs;\
                                                                const auto label1 = frame_buffer[code[++pc].value].value;\
                                                                const auto label2 = frame_buffer[code[++pc].value].value;\
                                                                const auto label3 = frame_buffer[code[++pc].value].value;\
                                                                if(_lhs.is_missing || _rhs.is_missing){GOTO(label3)}\
                                                                else if(_expr){GOTO(label1)}\
                                                                else{GOTO(label2)}\
                                                            }

#define OP_SIMPLE_BR_IN_RANGE_INST(_op, _arg1, _arg2, _arg3, _arg4, _expr) _L_##_op:{\
                                                                                const auto arg1 = _arg1;\
                                                                                const auto arg2 = _arg2;\
                                                                                const auto arg3 = _arg3;\
                                                                                const auto arg4 = _arg4;\
                                                                                const auto label1 = frame_buffer[code[++pc].value].value;\
                                                                                const auto label2 = frame_buffer[code[++pc].value].value;\
                                                                                const auto label3 = frame_buffer[code[++pc].value].value;\
                                                                                if(_arg1.is_missing || _arg2.is_missing || _arg3.is_missing || _arg4.is_missing){GOTO(label3)}\
                                                                                else if(_expr){GOTO(label1)}\
                                                                                else{GOTO(label2)}\
                                                                            }

/* ---- Calls --------------------------------------- */

/* ---- Heap objects: arrays ---------------------------------------------*/
#define OP_SIMPLE_GET_INT64FLOAT_INST(_op, _ptr, _offset, _vm_type) _L_##_op:{\
                                                                        const auto ptr = _ptr;\
                                                                        const auto offset = _offset;\
                                                                        const auto result_ptr = frame_buffer + code[++pc].value;\
                                                                        result_ptr->type = _vm_type;\
                                                                        result_ptr->is_missing = ptr.is_missing || offset.is_missing || offset.value >= ptr.length;\
                                                                        result_ptr->value = result_ptr->is_missing ? 0 : *(int64_t*)(ptr.value + offset.value*ptr.element_size);\
                                                                        DISPATCH();\
                                                                    }
#define OP_SIMPLE_GET_PTR_INST(_op, _ptr, _offset) _L_##_op:{\
                                                        const auto ptr = _ptr;\
                                                        const auto offset = _offset;\
                                                        const auto result_ptr = frame_buffer + code[++pc].value;\
                                                        result_ptr->type = ValueType::VT_PTR;\
                                                        result_ptr->is_missing = ptr.is_missing || offset.is_missing || offset.value >= ptr.length;\
                                                        if(result_ptr->is_missing){\
                                                            result_ptr->value = 0;\
                                                            result_ptr->capacity = 0;\
                                                            result_ptr->length = 0;\
                                                            result_ptr->element_size = 0;\
                                                        }\
                                                        else{\
                                                            const auto offset_ptr = (char*)(ptr.value + offset.value*ptr.element_size);\
                                                            result_ptr->value = *(int64_t*)offset_ptr;\
                                                            result_ptr->capacity = *(int64_t*)(offset_ptr + sizeof(std::int64_t));\
                                                            result_ptr->length = *(int64_t*)(offset_ptr + 2*sizeof(std::int64_t));\
                                                            result_ptr->element_size = *(int16_t*)(offset_ptr + 2*sizeof(std::int64_t)+sizeof(std::int16_t));\
                                                        }\
                                                        DISPATCH();\
                                                    }

#define OP_SIMPLE_PTR_OFFSET_INST(_op, _ptr, _offset, _expr) _L_##_op:{\
                                                                const auto ptr = _ptr;\
                                                                const auto offset = _offset;\
                                                                const auto result_ptr = frame_buffer + code[++pc].value;\
                                                                result_ptr->type = ValueType::VT_PTR;\
                                                                result_ptr->value = _expr;\
                                                                result_ptr->capacity = std::max<std::int64_t>(0, ptr.capacity - offset.capacity*ptr.element_size);\
                                                                result_ptr->length = std::max<std::int64_t>(0, ptr.length - offset.length*ptr.element_size);\
                                                                result_ptr->element_size = ptr.element_size;\
                                                                result_ptr->is_missing = ptr.is_missing || offset.is_missing;\
                                                                DISPATCH();\
                                                            }
                                                            
#define OP_SIMPLE_SET_INT64FLOAT_INST(_op, _ptr, _offset, _value) _L_##_op:{\
                                                                      const auto ptr = _ptr;\
                                                                      const auto offset = _offset;\
                                                                      const auto value = _value;\
                                                                      if(ptr.is_missing || offset.is_missing || offset.value >= ptr.length){DISPATCH()}\
                                                                      (*(int64_t*)(ptr.value + offset.value*ptr.element_size)) = value.value;\
                                                                      DISPATCH();\
                                                                  }

#define OP_SIMPLE_SET_PTR_INST(_op, _ptr, _offset, _value) _L_##_op:{\
                                                               const auto ptr = _ptr;\
                                                               const auto offset = _offset;\
                                                               const auto value = _value;\
                                                               if(ptr.is_missing || offset.is_missing || offset.value >= ptr.length){DISPATCH();}\
                                                               const auto offset_ptr = (char*)(ptr.value + offset.value*ptr.element_size);\
                                                               *(int64_t*)offset_ptr = value.value;\
                                                               *(int64_t*)(offset_ptr + sizeof(std::int64_t)) = value.capacity;\
                                                               *(int64_t*)(offset_ptr + 2*sizeof(std::int64_t)) = value.length;\
                                                               *(int16_t*)(offset_ptr + 2*sizeof(std::int64_t)+sizeof(std::int16_t)) = value.element_size;\
                                                               DISPATCH();\
                                                           }
                                                    
#define OP_SIMPLE_PTR_GETTER_METHOD_INST(_op, _expr) _L_##_op:{\
                                                            const auto ptr = frame_buffer[code[++pc].value];\
                                                            const auto result_ptr = frame_buffer + code[++pc].value;\
                                                            result_ptr->value = _expr;\
                                                            result_ptr->type = ValueType::VT_I64;\
                                                            result_ptr->is_missing = ptr.is_missing;\
                                                            DISPATCH();\
                                                        }