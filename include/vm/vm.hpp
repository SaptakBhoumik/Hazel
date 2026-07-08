#pragma once
#include "value.hpp"
#include "opcode.hpp"
#include <cstddef>
#include <vector>

#pragma once
#include <cstdint>
namespace Hazel{
namespace Snap{
using  ExternFuncType = void (*)(StreamValue* args, StreamValue** ret_value);//ret_value is pointer to the stack frame

class StreamVM{
    std::vector<StreamValue> code;
    std::size_t frame_buffer_size;
    std::size_t max_function_call_depth;
    public:
    StreamVM(std::vector<StreamValue> code, std::size_t frame_buffer_size, std::size_t max_function_call_depth) noexcept;
};
}
namespace Trend{};
}