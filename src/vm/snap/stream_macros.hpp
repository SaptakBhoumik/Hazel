#pragma once

#define FIXED_POINT_FLOAT_SCALING_FACTOR 10000//Because 4 decimal places

#define INSERT(op) dispatch[(std::uint64_t)Opcode::op]=&&_L_##op;
#define DISPATCH() goto *dispatch[(std::uint64_t)code[++pc].value];
#define COLD_GOTO_BLOCK(op)  _L_##op: { \
    execute_stream_vm(code + pc, frame_buffer, traceback_func_loc_idx, func_call_loc_idx, func_stack_size); \
    DISPATCH(); \
}

/* ---- Loads ----------------------------------------- */
#define OP_SIMPLE_LOAD_INST(op, _value) _L_##op:{\
                                            const auto value = _value;\
                                            frame_buffer[code[++pc].value] = value;\
                                            DISPATCH();\
                                        }

/* ---- Binary Arithmetic Instructions ----------------------------------*/
#define OP_SIMPLE_BINARY_INST(op, _lhs, _rhs, expr) _L_##op:{\
                                            const auto lhs = _lhs;\
                                            const auto rhs = _rhs;\
                                            const auto ptr = frame_buffer + code[++pc].value;\
                                            ptr->value = expr;\
                                            ptr->type = lhs.type;\
                                            ptr->is_missing = lhs.is_missing || rhs.is_missing;\
                                            DISPATCH();\
                                        }

#define OP_SIMPLE_DIV_REM_INST(op, _lhs, _rhs, expr) _L_##op:{\
                                            const auto lhs = _lhs;\
                                            const auto rhs = _rhs;\
                                            const auto ptr = frame_buffer + code[++pc].value;\
                                            ptr->type = lhs.type;\
                                            ptr->is_missing = lhs.is_missing || rhs.is_missing || rhs.value == 0 || (lhs.value == INT64_MIN && rhs.value == -1);\
                                            ptr->value = ptr->is_missing ? 0 : expr;\
                                            DISPATCH();\
                                        }