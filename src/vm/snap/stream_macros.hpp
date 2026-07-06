#pragma once

#define FIXED_POINT_FLOAT_SCALING_FACTOR 10000//Because 4 decimal places

#define INSERT(op) dispatch[(std::uint64_t)Opcode::op]=&&_L_##op;
#define DISPATCH() goto *dispatch[(std::uint64_t)code[++pc].value];
#define COLD_GOTO_BLOCK(op)  _L_##op: { \
    execute_stream_vm(code + pc, frame_buffer + curr_stack_offset, traceback_func_loc_idx + curr_func_call_depth, func_call_loc_idx + curr_func_call_depth, func_stack_size + curr_func_call_depth); \
    DISPATCH(); \
}

/* ---- Loads ----------------------------------------- */
#define OP_SIMPLE_LOAD_INST(op, _value) _L_##op:{\
                                            const auto value = _value;\
                                            frame_buffer[code[++pc].value] = value;\
                                            DISPATCH();\
                                        }