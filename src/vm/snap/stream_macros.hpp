#pragma once

#define FIXED_POINT_FLOAT_SCALING_FACTOR 10000//Because 4 decimal places

#define INSERT(_op) hot_dispatch[(std::uint64_t)Opcode::_op]=&&_L_##_op;
#define COLD_INSERT(_op) cold_dispatch[(std::uint64_t)Opcode::_op]=&&_L_##_op;
#define INSERT_COLD(_op) hot_dispatch[(std::uint64_t)Opcode::_op]=&&___L_COLD_LABEL;
#define DISPATCH() goto *hot_dispatch[(std::uint64_t)code[++pc].value];
#define GOTO(_loc) pc = _loc; goto *hot_dispatch[(std::uint64_t)code[pc].value];

/* ---- Loads ----------------------------------------- */
#define OP_SIMPLE_LOAD_INST(_op, _value) _L_##_op:{\
                                            const auto value = _value;\
                                            frame_buffer[code[++pc].value] = value;\
                                            DISPATCH();\
                                        }

/* ---- Binary Arithmetic Instructions ----------------------------------*/
#define OP_SIMPLE_BINARY_INST(_op, _lhs, _rhs, _expr, _is_missing, _type) _L_##_op:{\
                                                                      const auto lhs = _lhs;\
                                                                      const auto rhs = _rhs;\
                                                                      const auto ptr = frame_buffer + code[++pc].value;\
                                                                      ptr->type = _type;\
                                                                      ptr->is_missing = _is_missing;\
                                                                      ptr->value = _expr;\
                                                                      DISPATCH();\
                                                                  }

#define OP_SIMPLE_DIV_REM_INST(_op, _lhs, _rhs, _expr, _is_missing, _type) _L_##_op:{\
                                                                               const auto lhs = _lhs;\
                                                                               const auto rhs = _rhs;\
                                                                               const auto ptr = frame_buffer + code[++pc].value;\
                                                                               ptr->type = _type;\
                                                                               ptr->is_missing = _is_missing;\
                                                                               ptr->value = ptr->is_missing ? 0 : _expr;\
                                                                               DISPATCH();\
                                                                           }

#define OP_SIMPLE_FLOAT_DIV_INST(_op, _lhs, _rhs, _expr, _is_missing, _type) _L_##_op:{\
                                                                                  const auto lhs = _lhs;\
                                                                                  const auto rhs = _rhs;\
                                                                                  const auto ptr = frame_buffer + code[++pc].value;\
                                                                                  ptr->type = _type;\
                                                                                  /*This operation happens in 128bit integer space. INT128_MIN can never happen if we multiply by 10000(too small) so no need to check*/\
                                                                                  ptr->is_missing = _is_missing;\
                                                                                  ptr->value = ptr->is_missing ? 0 : _expr;\
                                                                                  DISPATCH();\
                                                                              }

/* ---- Other Arithmetic Instructions ------- */
#define OP_SIMPLE_COLD_WEIGHED_ADD_INST(_op, _acc, _acc_value, _weight, _mul_expr, _is_acc_missing, _is_missing_expr, _type) _L_##_op:{\
                                                                                                                                 const auto loop_count = code[++pc].value;\
                                                                                                                                 const auto acc = _acc;\
                                                                                                                                 bool is_missing = _is_acc_missing;\
                                                                                                                                 std::int64_t value = _acc_value;\
                                                                                                                                 for(std::int64_t i = 0; i < loop_count; i++){\
                                                                                                                                     const auto weight = _weight;\
                                                                                                                                     const auto arg = frame_buffer[code[++pc].value];\
                                                                                                                                     value += _mul_expr;\
                                                                                                                                     is_missing |= _is_missing_expr;\
                                                                                                                                 }\
                                                                                                                                 const auto ptr = frame_buffer + code[++pc].value;\
                                                                                                                                 ptr->type = _type;/*This ptr-1 is the last arg*/\
                                                                                                                                 ptr->is_missing = is_missing;\
                                                                                                                                 ptr->value = value;\
                                                                                                                                 return pc;\
                                                                                                                             }


/* ---- Comparison Instruction ------------------*/
#define OP_SIMPLE_CMP_BINARY_INST(_op, _lhs, _rhs, _expr, _is_missing) _L_##_op:{\
                                                                           const auto lhs = _lhs;\
                                                                           const auto rhs = _rhs;\
                                                                           const auto ptr = frame_buffer + code[++pc].value;\
                                                                           ptr->type = ValueType::VT_I64;\
                                                                           ptr->is_missing = _is_missing;\
                                                                           ptr->value = _expr;\
                                                                           DISPATCH();\
                                                                       }

#define OP_SIMPLE_IN_RANGE_INST(_op, _arg1, _arg2, _arg3, _arg4, _expr, _is_missing) _L_##_op:{\
                                                                                        const auto arg1 = _arg1;\
                                                                                        const auto arg2 = _arg2;\
                                                                                        const auto arg3 = _arg3;\
                                                                                        const auto arg4 = _arg4;\
                                                                                        const auto ptr = frame_buffer + code[++pc].value;\
                                                                                        ptr->type = ValueType::VT_I64;\
                                                                                        ptr->is_missing = _is_missing;\
                                                                                        ptr->value = _expr;\
                                                                                        DISPATCH();\
                                                                                    }

#define OP_SIMPLE_COLD_IN_RANGE_INST(_op, _arg1, _arg2, _arg3, _arg4, _expr, _is_missing) _L_##_op:{\
                                                                                              const auto arg1 = _arg1;\
                                                                                              const auto arg2 = _arg2;\
                                                                                              const auto arg3 = _arg3;\
                                                                                              const auto arg4 = _arg4;\
                                                                                              const auto ptr = frame_buffer + code[++pc].value;\
                                                                                              ptr->type = ValueType::VT_I64;\
                                                                                              ptr->is_missing = _is_missing;\
                                                                                              ptr->value = _expr;\
                                                                                              return pc;\
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
#define OP_SIMPLE_BR_BINARY_CMP_INST(_op, _lhs, _rhs, _expr, _is_missing) _L_##_op:{\
                                                                              const auto lhs = _lhs;\
                                                                              const auto rhs = _rhs;\
                                                                              const auto label1 = code[++pc].value;\
                                                                              const auto label2 = code[++pc].value;\
                                                                              const auto label3 = code[++pc].value;\
                                                                              if(_is_missing){GOTO(label3)}\
                                                                              else if(_expr){GOTO(label1)}\
                                                                              else{GOTO(label2)}\
                                                                          }

#define OP_SIMPLE_BR_IN_RANGE_INST(_op, _arg1, _arg2, _arg3, _arg4, _expr, _is_missing) _L_##_op:{\
                                                                                            const auto arg1 = _arg1;\
                                                                                            const auto arg2 = _arg2;\
                                                                                            const auto arg3 = _arg3;\
                                                                                            const auto arg4 = _arg4;\
                                                                                            const auto label1 = code[++pc].value;\
                                                                                            const auto label2 = code[++pc].value;\
                                                                                            const auto label3 = code[++pc].value;\
                                                                                            if(_is_missing){GOTO(label3)}\
                                                                                            else if(_expr){GOTO(label1)}\
                                                                                            else{GOTO(label2)}\
                                                                                        }

#define OP_SIMPLE_COLD_BR_IN_RANGE_INST(_op, _arg1, _arg2, _arg3, _arg4, _expr, _is_missing) _L_##_op:{\
                                                                                                 const auto arg1 = _arg1;\
                                                                                                 const auto arg2 = _arg2;\
                                                                                                 const auto arg3 = _arg3;\
                                                                                                 const auto arg4 = _arg4;\
                                                                                                 const auto label1 = code[++pc].value;\
                                                                                                 const auto label2 = code[++pc].value;\
                                                                                                 const auto label3 = code[++pc].value;\
                                                                                                 /*The -1 is important because we expect the PC to be incrimented after we return from cold function*/\
                                                                                                 if(_is_missing){return label3 - 1;}\
                                                                                                 else if(_expr){return label1 - 1;}\
                                                                                                 else{return label2 - 1;}\
                                                                                             }

/* ---- Heap objects: arrays ---------------------------------------------*/
#define OP_SIMPLE_GET_INT64FLOAT_INST(_op, _ptr, _vm_type) _L_##_op:{\
                                                               const auto ptr = _ptr;\
                                                               const auto offset = code[++pc].value;\
                                                               const auto result_ptr = frame_buffer + code[++pc].value;\
                                                               result_ptr->type = _vm_type;\
                                                               result_ptr->is_missing = ptr.is_missing || offset >= ptr.length;\
                                                               result_ptr->value = result_ptr->is_missing ? 0 : *(int64_t*)(ptr.value + offset*ptr.element_size);\
                                                               DISPATCH();\
                                                           }
#define OP_SIMPLE_GET_PTR_INST(_op, _ptr) _L_##_op:{\
                                              const auto ptr = _ptr;\
                                              const auto offset = code[++pc].value;\
                                              const auto result_ptr = frame_buffer + code[++pc].value;\
                                              result_ptr->type = ValueType::VT_PTR;\
                                              result_ptr->is_missing = ptr.is_missing || offset >= ptr.length;\
                                              if(!result_ptr->is_missing){\
                                                  const auto offset_ptr = (char*)(ptr.value + offset*ptr.element_size);\
                                                  result_ptr->value = *(int64_t*)offset_ptr;\
                                                  result_ptr->capacity = *(int64_t*)(offset_ptr + sizeof(std::int64_t));\
                                                  result_ptr->length = *(int64_t*)(offset_ptr + 2*sizeof(std::int64_t));\
                                                  result_ptr->element_size = *(int16_t*)(offset_ptr + 2*sizeof(std::int64_t)+sizeof(std::int16_t));\
                                              }\
                                              DISPATCH();\
                                          }

#define OP_SIMPLE_COLD_GET_INT64FLOAT_INST(_op, _ptr, _vm_type) _L_##_op:{\
                                                                    const auto ptr = _ptr;\
                                                                    const auto offset = frame_buffer[code[++pc].value];\
                                                                    const auto result_ptr = frame_buffer + code[++pc].value;\
                                                                    result_ptr->type = _vm_type;\
                                                                    result_ptr->is_missing = ptr.is_missing || offset.is_missing || offset.value >= ptr.length;\
                                                                    result_ptr->value = result_ptr->is_missing ? 0 : *(int64_t*)(ptr.value + offset.value*ptr.element_size);\
                                                                    return pc;\
                                                                }
#define OP_SIMPLE_COLD_GET_PTR_INST(_op, _ptr) _L_##_op:{\
                                                   const auto ptr = _ptr;\
                                                   const auto offset = frame_buffer[code[++pc].value];\
                                                   const auto result_ptr = frame_buffer + code[++pc].value;\
                                                   result_ptr->type = ValueType::VT_PTR;\
                                                   result_ptr->is_missing = ptr.is_missing || offset.is_missing || offset.value >= ptr.length;\
                                                   if(!result_ptr->is_missing){\
                                                       const auto offset_ptr = (char*)(ptr.value + offset.value*ptr.element_size);\
                                                       result_ptr->value = *(int64_t*)offset_ptr;\
                                                       result_ptr->capacity = *(int64_t*)(offset_ptr + sizeof(std::int64_t));\
                                                       result_ptr->length = *(int64_t*)(offset_ptr + 2*sizeof(std::int64_t));\
                                                       result_ptr->element_size = *(int16_t*)(offset_ptr + 2*sizeof(std::int64_t)+sizeof(std::int16_t));\
                                                   }\
                                                   return pc;\
                                               }

#define OP_SIMPLE_COLD_PTR_OFFSET_INST(_op, _ptr, _offset, _offset_val, _expr, _is_missing) _L_##_op:{\
                                                                                                const auto ptr = _ptr;\
                                                                                                const auto offset = _offset;\
                                                                                                const auto result_ptr = frame_buffer + code[++pc].value;\
                                                                                                result_ptr->type = ValueType::VT_PTR;\
                                                                                                result_ptr->value = _expr;\
                                                                                                result_ptr->capacity = std::max<std::int64_t>(0, ptr.capacity - _offset_val);\
                                                                                                result_ptr->length = std::max<std::int64_t>(0, ptr.length - _offset_val);\
                                                                                                result_ptr->element_size = ptr.element_size;\
                                                                                                result_ptr->is_missing = _is_missing;\
                                                                                                return pc;\
                                                                                            }

#define OP_SIMPLE_SET_INT64FLOAT_INST(_op, _ptr, _value) _L_##_op:{\
                                                             const auto ptr = _ptr;\
                                                             const auto offset = code[++pc].value;\
                                                             const auto value = _value;\
                                                             if(!ptr.is_missing && !value.is_missing && offset < ptr.length){\
                                                                 (*(int64_t*)(ptr.value + offset*ptr.element_size)) = value.value;\
                                                             }\
                                                             DISPATCH();\
                                                         }

#define OP_SIMPLE_SET_PTR_INST(_op, _ptr, _value) _L_##_op:{\
                                                      const auto ptr = _ptr;\
                                                      const auto offset = code[++pc].value;\
                                                      const auto value = _value;\
                                                      if(!ptr.is_missing && !value.is_missing && offset < ptr.length){\
                                                          const auto offset_ptr = (char*)(ptr.value + offset*ptr.element_size);\
                                                          *(int64_t*)offset_ptr = value.value;\
                                                          *(int64_t*)(offset_ptr + sizeof(std::int64_t)) = value.capacity;\
                                                          *(int64_t*)(offset_ptr + 2*sizeof(std::int64_t)) = value.length;\
                                                          *(int16_t*)(offset_ptr + 2*sizeof(std::int64_t)+sizeof(std::int16_t)) = value.element_size;\
                                                      }\
                                                      DISPATCH();\
                                                  }
#define OP_SIMPLE_COLD_SET_INT64FLOAT_INST(_op, _ptr, _value) _L_##_op:{\
                                                                  const auto ptr = _ptr;\
                                                                  const auto offset = frame_buffer[code[++pc].value];\
                                                                  const auto value = _value;\
                                                                  if(!ptr.is_missing && !offset.is_missing && !value.is_missing && offset.value < ptr.length){\
                                                                      (*(int64_t*)(ptr.value + offset.value*ptr.element_size)) = value.value;\
                                                                  }\
                                                                  return pc;\
                                                              }

#define OP_SIMPLE_COLD_SET_PTR_INST(_op, _ptr, _value) _L_##_op:{\
                                                           const auto ptr = _ptr;\
                                                           const auto offset = frame_buffer[code[++pc].value];\
                                                           const auto value = _value;\
                                                           if(!ptr.is_missing && !offset.is_missing && !value.is_missing && offset.value < ptr.length){\
                                                               const auto offset_ptr = (char*)(ptr.value + offset.value*ptr.element_size);\
                                                               *(int64_t*)offset_ptr = value.value;\
                                                               *(int64_t*)(offset_ptr + sizeof(std::int64_t)) = value.capacity;\
                                                               *(int64_t*)(offset_ptr + 2*sizeof(std::int64_t)) = value.length;\
                                                               *(int16_t*)(offset_ptr + 2*sizeof(std::int64_t)+sizeof(std::int16_t)) = value.element_size;\
                                                           }\
                                                           return pc;\
                                                       }
      
#define OP_SIMPLE_COLD_PTR_GETTER_METHOD_INST(_op, _expr) _L_##_op:{\
                                                              const auto ptr = frame_buffer[code[++pc].value];\
                                                              const auto result_ptr = frame_buffer + code[++pc].value;\
                                                              result_ptr->value = _expr;\
                                                              result_ptr->type = ValueType::VT_I64;\
                                                              result_ptr->is_missing = ptr.is_missing;\
                                                              return pc;\
                                                          }

/*----Time series related--------------------------------------*/
#define OP_SIMPLE_COLD_BINARY_FIND_TYPE1_INST(_op, _ptr, _start_idx, _size, _value, _expr, _missing_condition) _L_##_op:{\
                                                                                                                   const auto ptr = _ptr;\
                                                                                                                   const auto start_idx = _start_idx;\
                                                                                                                   const auto size = _size;\
                                                                                                                   const auto value = _value;\
                                                                                                                   const auto result_ptr = frame_buffer + code[++pc].value;\
                                                                                                                   result_ptr->type = ValueType::VT_I64;\
                                                                                                                   result_ptr->is_missing = ptr.is_missing || start_idx.is_missing || size.is_missing || value.is_missing || start_idx.value >= ptr.length || start_idx.value*ptr.element_size + size.value*sizeof(std::int64_t) > ptr.length*ptr.element_size || size.value <= 0;\
                                                                                                                   result_ptr->value = result_ptr->is_missing ? 0 : _expr;\
                                                                                                                   result_ptr->is_missing |= _missing_condition;\
                                                                                                                   return pc;\
                                                                                                               }

#define OP_SIMPLE_COLD_BINARY_FIND_TYPE2_INST(_op, _ptr, _start_idx, _value, _expr, _missing_condition) _L_##_op:{\
                                                                                                            const auto ptr = _ptr;\
                                                                                                            const auto start_idx = _start_idx;\
                                                                                                            const auto size = ((_ptr.length - start_idx.value)*ptr.element_size)/sizeof(std::int64_t);\
                                                                                                            const auto value = _value;\
                                                                                                            const auto result_ptr = frame_buffer + code[++pc].value;\
                                                                                                            result_ptr->type = ValueType::VT_I64;\
                                                                                                            result_ptr->is_missing = ptr.is_missing || start_idx.is_missing || value.is_missing || start_idx.value >= ptr.length || start_idx.value*ptr.element_size + size*sizeof(std::int64_t) > ptr.length*ptr.element_size || size <= 0;\
                                                                                                            result_ptr->value = result_ptr->is_missing ? 0 : _expr;\
                                                                                                            result_ptr->is_missing |= _missing_condition;\
                                                                                                            return pc;\
                                                                                                        }